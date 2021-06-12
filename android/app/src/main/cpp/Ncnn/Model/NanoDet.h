//
// Created by joes on 6/06/21.
//

#include <net.h>
#include <opencv2/core/mat.hpp>
#include "../Common/Common.h"
#include "../../Common/Utils.h"

#ifndef OPENDASHCAM_NANODET_H
#define OPENDASHCAM_NANODET_H

#endif //OPENDASHCAM_NANODET_H

struct NanoDetOut
{
    std::vector<RectDetection> result;
    object_rect effect_area;
};


class NanoDet
{
public:
    NanoDet(AAssetManager *mgr, const char *param, const char *bin, bool useGPU);
    ~NanoDet();
    NanoDetOut detect(cv::Mat image, float score_threshold, float nms_threshold);
    void Draw(cv::Mat& image, const std::vector<RectDetection>& bboxes, const object_rect effect_area);

    static NanoDet *detector;

private:
    // Method
    void PreProcess();
    void Process();
    void PostProcess();
    void decode_infer(ncnn::Mat& cls_pred, ncnn::Mat& dis_pred, int stride);
    void resize_uniform();
    RectDetection disPred2Bbox(const float*& dfl_det, int label, float score, int x, int y, int stride);
    static void nms(std::vector<RectDetection>& result, float nms_threshold);

    float score_threshold = {default_score_threshold};
    float nms_threshold = {default_nms_threshold};

    // Processed Data (This will be shared among the functions)
    cv::Mat cv_input_image;
    cv::Mat cv_resized_input_image;
    ncnn::Mat ncnn_input_image;
    std::vector<std::vector<RectDetection>> temp_results;
    std::vector<RectDetection> final_result;

    object_rect effect_area{};
    ncnn::Net* Net;
    bool hasGPU {false};



    // Constant Value
    const int input_size[2] = {320, 320};
    const int num_class = 80;
    const int reg_max = 7;
    const cv::Size dst_size = {cv::Size(320, 320)};
    const float mean_vals[3] = { 103.53f, 116.28f, 123.675f };
    const float norm_vals[3] = { 0.017429f, 0.017507f, 0.017125f };
    const std::vector<HeadInfo> heads_info{
            // cls_pred|dis_pred|stride
            {"cls_pred_stride_8", "dis_pred_stride_8", 8},
            {"cls_pred_stride_16", "dis_pred_stride_16", 16},
            {"cls_pred_stride_32", "dis_pred_stride_32", 32},
    };
};

