#ifndef TRT_H
#define TRT_H


#include <fstream>
#include "json.h"
#include <assert.h>
#include "NvOnnxParser.h"
#include "NvOnnxConfig.h"
#include "calibrator.h"
#include "UpsamplePlugin.h"
#include "yololayer.h"
#include "hardswish.h"


struct Param{
    int input_c;
    int input_h;
    int input_w;
    bool createENG;
    string ENGPath;
    bool fp16;
    bool int8;
    bool Div_255;
    string cali_txt;
    string cali_table;
    vector<float> mean;
    vector<float> std;
    string weightPath;
    string wtsAllPath;
    string onnxPath;
    string inputBlobName;
    //string outputBlobName;
    int maxBatchsize;
    int outputSize = 0;
    bool doInfer;
    int BatchSize;
    string imgDir;
    string imgType;
    Json::Value layers;
    vector<string> outputNames;
};

class Logger:public nvinfer1::ILogger
{
public:
    void log(nvinfer1::ILogger::Severity severity, const char *msg) override
    {
        // suppress info-level messages
        if (severity == Severity::kINFO)
            return;

        switch (severity)
        {
        case Severity::kINTERNAL_ERROR:
            std::cerr << "INTERNAL_ERROR: ";
            break;
        case Severity::kERROR:
            std::cerr << "ERROR: ";
            break;
        case Severity::kWARNING:
            std::cerr << "WARNING: ";
            break;
        case Severity::kINFO:
            std::cerr << "INFO: ";
            break;
        default:
            std::cerr << "UNKNOWN: ";
            break;
        }
        std::cerr << msg << std::endl;
    }
};

class trt
{
public:
    trt(const string &jsonPath);
    ~trt();
    void debug_print(nvinfer1::ITensor *input_tensor,const string &head);
    void printWeight(Weights wts, int wtsSize);
    vector<float> loadWeights(const string &filePath);
    void createENG();
    void onnx2trt();
    void addLayer(Json::Value layer);
    void inference_init(int batchsize);
    void doInference(const float *input, int batchsize, float *output);
    void doInference_int(const float *input, int batchsize, int *output);
    ITensor* trt_convNet(ITensor* input,string weightsPath,string biasFile,
                         int output_c,DimsHW kernel,DimsHW stride = DimsHW{1,1},
                         DimsHW padding =DimsHW{0,0},DimsHW dilations =DimsHW{1,1},
                         int groups = 1,bool pre = false,bool post = false);
    ITensor* trt_deconvNet(ITensor* input,string weightsPath,string biasFile,
                         int output_c,DimsHW kernel,DimsHW stride = DimsHW{1,1},
                         DimsHW padding =DimsHW{0,0},DimsHW dilations =DimsHW{1,1},
                         int groups = 1,bool pre = false,bool post = false);
    ITensor* trt_bnNet(ITensor* input, string weightsPath,float eps=1.0e-5);
    ITensor* trt_activeNet(ITensor* input,string acti_type,float alpha=0.0,float beta=0.0);
    ITensor* trt_poolNet(ITensor* input,string pooltype,DimsHW kernel,DimsHW stride,DimsHW padding);
    ITensor* trt_eltNet(ITensor* input1,ITensor* input2,string elt_Type);
    ITensor* conv_bn_active(ITensor* input,string weightsPath, int output_c,int k,int s=1,int p=0,int dilations=1,int groups=1,
                            float eps=1e-5, string active_type="relu", float alpha=0.0, float beta=0.0);
    ITensor* trt_resnetCBA(Json::Value temp,ITensor* input);
    void trt_preInput(Json::Value layer);
    void trt_conv(Json::Value layer);
    void trt_deconv(Json::Value layer);
    void trt_padding(Json::Value layer);
    void trt_bn(Json::Value layer);
    void trt_active(Json::Value layer);
    void trt_pool(Json::Value layer);
    void trt_Pool(Json::Value layer);
    void trt_elt(Json::Value layer);
    void trt_fc(Json::Value layer);
    void trt_concat(Json::Value layer);
    void trt_slice(Json::Value layer);
    void trt_softmax(Json::Value layer);
    void trt_shuffle(Json::Value layer);
    void trt_matmul(Json::Value layer);
    void trt_topk(Json::Value layer);
    void trt_reduce(Json::Value layer);
    void trt_constant(Json::Value layer);
    void trt_pReLU(Json::Value layer);
    void trt_convBnActive(Json::Value layer);
    void trt_resnetLayer(Json::Value layer);
    void trt_resnet3(Json::Value layer);
    void trt_focus(Json::Value layer);
    void trt_UpSample(Json::Value layer);
    void trt_UpSample_plugin(Json::Value layer);
    void trt_groupNorm(Json::Value layer);
    void trt_unary(Json::Value layer);
    ITensor* convBlock(ITensor* input,int outch,int k,int s,string lname,string acti_type,
                       float eps=1e-3,float alpha = 0.0);
    ITensor* bottleneck(ITensor* input, string lname,string acti_type,int c1, int c2, bool shortcut, float e,
                        float eps=1e-3,float alpha = 0.0);
    ITensor* SPP();
    void yolo_C3(Json::Value layer);
    void trt_yolo(Json::Value layer);
    void yolo_spp(Json::Value layer);
    int out_channel(int &out_c, int &n);
    void resnet(Json::Value layer);
    void hrnet_res(Json::Value layer);
    void hrnet_up(Json::Value layer);




    Param param;
private:
    Logger m_logger;

    map<string,ITensor*> Layers;
    INetworkDefinition *m_Network; //network
    vector<void *> m_bindings;
    void* temp;
    vector<int> outputs;
    nvinfer1::IExecutionContext *m_context;
    cudaStream_t m_cudaStream;
    nvinfer1::ICudaEngine *m_engine;
    int inputIndex;
    int outputIndex;
};

#endif // TRT_H
