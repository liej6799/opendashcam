#include "NanoDet.h"
#include <benchmark.h>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <utility>

NanoDet *NanoDet::detector = nullptr;

inline float fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}

template<typename Tp>
int activation_function_softmax(const Tp* src, Tp* dst, int length)
{
    const Tp alpha = *std::max_element(src, src + length);
    Tp denominator{ 0 };

    for (int i = 0; i < length; ++i) {
        dst[i] = fast_exp(src[i] - alpha);
        denominator += dst[i];
    }

    for (int i = 0; i < length; ++i) {
        dst[i] /= denominator;
    }

    return 0;
}

NanoDet::NanoDet(AAssetManager *mgr, const char *param, const char *bin, bool useGPU)
{
    Net = new ncnn::Net();
#if NCNN_VULKAN
    hasGPU = ncnn::get_gpu_count() > 0;
#endif
    Net->opt.use_vulkan_compute = hasGPU && useGPU;
    Net->opt.use_fp16_arithmetic = true;
    Net->opt.lightmode = false;
    Net->opt.num_threads = 4;
    Net->load_param(mgr, param);
    Net->load_model(mgr, bin);
}

NanoDet::~NanoDet()
{
    delete this->Net;
}


void NanoDet::resize_uniform()
{
    int w = cv_input_image.cols;
    int h = cv_input_image.rows;
    int dst_w = dst_size.width;
    int dst_h = dst_size.height;
    cv_resized_input_image = cv::Mat(cv::Size(dst_w, dst_h), CV_8UC3, cv::Scalar(0));

    float ratio_src = w * 1.0 / h;
    float ratio_dst = dst_w * 1.0 / dst_h;

    int tmp_w = 0;
    int tmp_h = 0;
    if (ratio_src > ratio_dst) {
        tmp_w = dst_w;
        tmp_h = floor((dst_w * 1.0 / w) * h);
    }
    else if (ratio_src < ratio_dst) {
        tmp_h = dst_h;
        tmp_w = floor((dst_h * 1.0 / h) * w);
    }
    else {
        cv::resize(cv_input_image, cv_resized_input_image, dst_size);
        effect_area.x = 0;
        effect_area.y = 0;
        effect_area.width = dst_w;
        effect_area.height = dst_h;
    }

    cv::Mat tmp;
    cv::resize(cv_input_image, tmp, cv::Size(tmp_w, tmp_h));

    if (tmp_w != dst_w) {
        int index_w = floor((dst_w - tmp_w) / 2.0);
        for (int i = 0; i < dst_h; i++) {
            memcpy(cv_resized_input_image.data + i * dst_w * 3 + index_w * 3, tmp.data + i * tmp_w * 3, tmp_w * 3);
        }
        effect_area.x = index_w;
        effect_area.y = 0;
        effect_area.width = tmp_w;
        effect_area.height = tmp_h;
    }
    else if (tmp_h != dst_h) {
        int index_h = floor((dst_h - tmp_h) / 2.0);
        memcpy(cv_resized_input_image.data + index_h * dst_w * 3, tmp.data, tmp_w * tmp_h * 3);
        effect_area.x = 0;
        effect_area.y = index_h;
        effect_area.width = tmp_w;
        effect_area.height = tmp_h;
    }
    else {
        printf("error\n");
    }
}

void NanoDet::PreProcess()
{
    resize_uniform();
    int img_w = cv_resized_input_image.cols;
    int img_h = cv_resized_input_image.rows;

    ncnn_input_image = ncnn::Mat::from_pixels(cv_resized_input_image.data, ncnn::Mat::PIXEL_BGR, img_w, img_h);
    ncnn_input_image.substract_mean_normalize(mean_vals, norm_vals);
}

void NanoDet::Process()
{
    ncnn::Extractor extractor = this->Net->create_extractor();
    extractor.input("input.1", ncnn_input_image);

    temp_results.clear();
    temp_results.resize(this->num_class);

    for (const auto& head_info : this->heads_info)
    {
        ncnn::Mat dis_pred;
        ncnn::Mat cls_pred;
        extractor.extract(head_info.dis_layer.c_str(), dis_pred);
        extractor.extract(head_info.cls_layer.c_str(), cls_pred);
        this->decode_infer(cls_pred, dis_pred, head_info.stride);
    }
}

void NanoDet::PostProcess()
{
    final_result.clear();

    for (auto & temp_result : temp_results) {

        NanoDet::nms(temp_result, nms_threshold);
        for (auto box : temp_result) {
            final_result.push_back(box);
        }
    }
}
void NanoDet::Draw(cv::Mat& image, const std::vector<RectDetection>& bboxes, const object_rect effect_area) {
    int src_w = image.cols;
    int src_h = image.rows;
    int dst_w = effect_area.width;
    int dst_h = effect_area.height;
    float width_ratio = (float)src_w / (float)dst_w;
    float height_ratio = (float)src_h / (float)dst_h;

    for (size_t i = 0; i < bboxes.size(); i++)
    {
        const RectDetection& bbox = bboxes[i];
        __android_log_print(ANDROID_LOG_DEBUG, "Result", "%f ", (bbox.bbox.tl().x - effect_area.x) * width_ratio);


        cv::Scalar color = cv::Scalar(color_list[bbox.class_idx][0], color_list[bbox.class_idx][1], color_list[bbox.class_idx][2]);

        cv::rectangle(image, cv::Rect(cv::Point((bbox.bbox.tl().x - effect_area.x) * width_ratio, (bbox.bbox.tl().y - effect_area.y) * height_ratio),
                                      cv::Point((bbox.bbox.br().x - effect_area.x) * width_ratio, (bbox.bbox.br().y - effect_area.y) * height_ratio)),  color, 5);
    }
}


NanoDetOut NanoDet::detect(cv::Mat image, float score_threshold, float nms_threshold)
{
    NanoDet::score_threshold = score_threshold;
    NanoDet::nms_threshold = nms_threshold;
    cv_input_image = std::move(image);

    PreProcess();
    Process();
    PostProcess();
    return NanoDetOut{final_result, effect_area};
}

void NanoDet::decode_infer(ncnn::Mat& cls_pred, ncnn::Mat& dis_pred, int stride)
{
    int feature_h = this->input_size[1] / stride;
    int feature_w = this->input_size[0] / stride;

    for (int idx = 0; idx < feature_h * feature_w; idx++)
    {
        const float* scores = cls_pred.row(idx);

        int row = idx / feature_w;
        int col = idx % feature_w;
        float score = 0;
        int cur_label = 0;
        for (int label = 0; label < this->num_class; label++)
        {
            if (scores[label] > score)
            {
                score = scores[label];
                cur_label = label;
            }
        }

        if (score > NanoDet::score_threshold)
        {
            __android_log_print(ANDROID_LOG_DEBUG, "YoloV5Ncnn", "%f", score);
            const float* bbox_pred = dis_pred.row(idx);
            temp_results[cur_label].push_back(this->disPred2Bbox(bbox_pred, cur_label, score, col, row, stride));
        }

    }
}

RectDetection NanoDet::disPred2Bbox(const float*& dfl_det, int label, float score, int x, int y, int stride)
{
    float ct_x = (x + 0.5) * stride;
    float ct_y = (y + 0.5) * stride;
    std::vector<float> dis_pred;
    dis_pred.resize(4);
    for (int i = 0; i < 4; i++)
    {
        float dis = 0;
        float* dis_after_sm = new float[this->reg_max + 1];
        activation_function_softmax(dfl_det + i * (this->reg_max + 1), dis_after_sm, this->reg_max + 1);
        for (int j = 0; j < this->reg_max + 1; j++)
        {
            dis += j * dis_after_sm[j];
        }
        dis *= stride;
        //std::cout << "dis:" << dis << std::endl;
        dis_pred[i] = dis;
        delete[] dis_after_sm;
    }
    float xmin = (std::max)(ct_x - dis_pred[0], .0f);
    float ymin = (std::max)(ct_y - dis_pred[1], .0f);
    float xmax = (std::min)(ct_x + dis_pred[2], (float)this->input_size[0]);
    float ymax = (std::min)(ct_y + dis_pred[3], (float)this->input_size[1]);

    //std::cout << xmin << "," << ymin << "," << xmax << "," << xmax << "," << std::endl;
    cv::Rect rect (cv::Point(xmin, ymin), cv::Point(xmax, ymax));

    return RectDetection { rect, score, label };
}

void NanoDet::nms(std::vector<RectDetection>& input_boxes, float NMS_THRESH)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](RectDetection a, RectDetection b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).bbox.br().x - input_boxes.at(i).bbox.tl().x + 1)
                   * (input_boxes.at(i).bbox.br().y - input_boxes.at(i).bbox.tl().y + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        for (int j = i + 1; j < int(input_boxes.size());) {
            float xx1 = (std::max)(input_boxes[i].bbox.tl().x, input_boxes[j].bbox.tl().x);
            float yy1 = (std::max)(input_boxes[i].bbox.tl().y, input_boxes[j].bbox.tl().y);
            float xx2 = (std::min)(input_boxes[i].bbox.br().x, input_boxes[j].bbox.br().x);
            float yy2 = (std::min)(input_boxes[i].bbox.br().y, input_boxes[j].bbox.br().y);
            float w = (std::max)(float(0), xx2 - xx1 + 1);
            float h = (std::max)(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else {
                j++;
            }
        }
    }
}

