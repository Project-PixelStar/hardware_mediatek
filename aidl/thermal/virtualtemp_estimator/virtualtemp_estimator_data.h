/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstddef>
#include <mutex>
#include <string>

#pragma once

namespace thermal {
namespace vtestimator {

// Current version only supports single input/output tensors
constexpr int kNumInputTensors = 1;
constexpr int kNumOutputTensors = 1;

typedef void *(*tflitewrapper_create)(int num_input_tensors, int num_output_tensors);
typedef bool (*tflitewrapper_init)(void *handle, const char *model_path);
typedef bool (*tflitewrapper_invoke)(void *handle, float *input_samples, int num_input_samples,
                                     float *output_samples, int num_output_samples);
typedef void (*tflitewrapper_destroy)(void *handle);

struct TFLiteWrapperMethods {
    tflitewrapper_create create;
    tflitewrapper_init init;
    tflitewrapper_invoke invoke;
    tflitewrapper_destroy destroy;
    mutable std::mutex mutex;
};

struct VtEstimatorTFLiteData {
    VtEstimatorTFLiteData(size_t num_input_samples) {
        input_buffer = new float[num_input_samples];
        input_buffer_size = num_input_samples;
        is_initialized = false;
        tflite_wrapper = nullptr;

        tflite_methods.create = nullptr;
        tflite_methods.init = nullptr;
        tflite_methods.invoke = nullptr;
        tflite_methods.destroy = nullptr;
    }

    void *tflite_wrapper;
    float *input_buffer;
    size_t input_buffer_size;
    std::string model_path;
    TFLiteWrapperMethods tflite_methods;
    bool is_initialized;

    ~VtEstimatorTFLiteData() {
        if (tflite_wrapper && tflite_methods.destroy) {
            tflite_methods.destroy(tflite_wrapper);
        }

        if (input_buffer) {
            delete input_buffer;
        }
    }
};

struct VtEstimatorLinearModelData {
    VtEstimatorLinearModelData(size_t num_input_sensors) {
        num_linked_sensors = num_input_sensors;
        prev_samples_order = 1;
        cur_sample_index = 0;
        first_iteration = true;
        offset = 0;
        is_initialized = false;
        use_prev_samples = false;
    }

    ~VtEstimatorLinearModelData() {}

    size_t num_linked_sensors;
    size_t prev_samples_order;
    size_t cur_sample_index;
    std::vector<std::vector<float>> input_samples;
    std::vector<std::vector<float>> coefficients;
    float offset;
    bool use_prev_samples;
    bool first_iteration;
    bool is_initialized;
    mutable std::mutex mutex;
};

}  // namespace vtestimator
}  // namespace thermal