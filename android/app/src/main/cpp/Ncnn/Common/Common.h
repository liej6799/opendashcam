//
// Created by joes on 6/06/21.
//

#ifndef OPENDASHCAM_COMMON_H
#define OPENDASHCAM_COMMON_H

const static float default_score_threshold=  0.4;
const static float default_nms_threshold = 0.5;


struct HeadInfo
{
    std::string cls_layer;
    std::string dis_layer;
    int stride;
};

#endif //OPENDASHCAM_COMMON_H
