#include "mathtools/convolution_std.h"

namespace STD_Convolution
{
    void init_workspace(Workspace& ws, Convolution_Mode mode, int h_src, int w_src, int h_kernel, int w_kernel)
    {
        ws.h_src = h_src;
        ws.w_src = w_src;
        ws.h_kernel = h_kernel;
        ws.w_kernel = w_kernel;
        ws.mode = mode;

        switch (mode)
        {
        case LINEAR_FULL:
            ws.h_dst = ws.h_src + ws.h_kernel - 1;
            ws.w_dst = ws.w_src + ws.w_kernel - 1;
            break;
        case LINEAR_SAME:
            ws.h_dst = ws.h_src;
            ws.w_dst = ws.w_src;
            break;
        case LINEAR_VALID:
            ws.h_dst = ws.h_src - ws.h_kernel + 1;
            ws.w_dst = ws.w_src - ws.w_kernel + 1;
            break;
        case CIRCULAR_SAME:
            ws.h_dst = ws.h_src;
            ws.w_dst = ws.w_src;
            break;
        case CIRCULAR_FULL:
            ws.h_dst = ws.h_src + ws.h_kernel - 1;
            ws.w_dst = ws.w_src + ws.w_kernel - 1;
            break;
        default:
            printf("Unrecognized mode, valid modes are :\n");
            printf(" LINEAR_FULL \n");
            printf(" LINEAR_SAME \n");
            printf(" LINEAR_VALID \n");
            printf(" CIRCULAR_SAME \n");
            printf(" CIRCULAR_FULL \n");
        }

        if (ws.h_dst > 0 && ws.w_dst > 0)
            ws.dst = new double[ws.h_dst * ws.w_dst];
        //else
        //  printf("Warning : The result is an empty matrix !\n");

    }

    void clear_workspace(Workspace& ws)
    {
        if (ws.h_dst > 0 && ws.w_dst > 0)
            delete[] ws.dst;
    }

    void update_workspace(Workspace& ws, Convolution_Mode mode, int h_src, int w_src, int h_kernel, int w_kernel)
    {
        clear_workspace(ws);
        init_workspace(ws, mode, h_src, w_src, h_kernel, w_kernel);
    }

    void convolve(Workspace& ws, double* src, double* kernel)
    {
        double temp;
        int i, j, k, l;
        int low_k, high_k, low_l, high_l;
        int i_src, j_src;

        if (ws.h_dst <= 0 || ws.w_dst <= 0)
            return;

        switch (ws.mode)
        {
        case LINEAR_FULL:
            // Full linear convolution of size N + M -1
            for (i = 0; i < ws.h_dst; ++i)
            {
                low_k = std::max(0, i - ws.h_kernel + 1);
                high_k = std::min(ws.h_src - 1, i);
                for (j = 0; j < ws.w_dst; ++j)
                {
                    low_l = std::max(0, j - ws.w_kernel + 1);
                    high_l = std::min(ws.w_src - 1, j);
                    temp = 0.0;
                    for (k = low_k; k <= high_k; ++k)
                    {
                        for (l = low_l; l <= high_l; ++l)
                        {
                            temp += src[k * ws.w_src + l] * kernel[(i - k) * ws.w_kernel + (j - l)];
                        }
                    }
                    ws.dst[i * ws.w_dst + j] = temp;
                }
            }
            break;
        case LINEAR_SAME:
            // Same linear convolution, of size N
            for (i = 0; i < ws.h_dst; ++i)
            {
                low_k = std::max(0, i - int((ws.h_kernel - 1.0) / 2.0));
                high_k = std::min(ws.h_src - 1, i + int(ws.h_kernel / 2.0));
                for (j = 0; j < ws.w_dst; ++j)
                {
                    low_l = std::max(0, j - int((ws.w_kernel - 1.0) / 2.0));
                    high_l = std::min(ws.w_src - 1, j + int(ws.w_kernel / 2.0));
                    temp = 0.0;
                    for (k = low_k; k <= high_k; ++k)
                    {
                        for (l = low_l; l <= high_l; ++l)
                        {
                            temp += src[k * ws.w_src + l] * kernel[(i - k + int(ws.h_kernel / 2.0)) * ws.w_kernel + (j - l + int(ws.w_kernel / 2.0))];
                        }
                    }
                    ws.dst[i * ws.w_dst + j] = temp;
                }
            }
            break;
        case LINEAR_VALID:
            // Valid linear convolution, of size N - M
            for (i = 0; i < ws.h_dst; ++i)
            {
                for (j = 0; j < ws.w_dst; ++j)
                {
                    temp = 0.0;
                    for (k = i; k <= i + ws.h_kernel - 1; ++k)
                    {
                        for (l = j; l <= j + ws.w_kernel - 1; ++l)
                        {
                            temp += src[k * ws.w_src + l] * kernel[(i + ws.h_kernel - 1 - k) * ws.w_kernel + (j + ws.w_kernel - 1 - l)];
                        }
                    }
                    ws.dst[i * ws.w_dst + j] = temp;
                }
            }
            break;
        case CIRCULAR_SAME:
        case CIRCULAR_FULL:
            // We suppose the filter has a size at most the size of the image
            for (i = 0; i < ws.h_dst; ++i)
            {
                for (j = 0; j < ws.w_dst; ++j)
                {
                    temp = 0.0;
                    // We browse the kernel
                    for (k = 0; k < ws.h_kernel; ++k)
                    {
                        i_src = (i - k) % ws.h_dst;
                        if (i_src < 0)
                            i_src += ws.h_dst;

                        for (l = 0; l < ws.w_kernel; ++l)
                        {
                            j_src = (j - l) % ws.w_dst;
                            if (j_src < 0)
                                j_src += ws.w_dst;
                            if (i_src >= 0 && i_src < ws.h_src
                                && j_src >= 0 && j_src < ws.w_src)
                                temp += src[i_src * ws.w_src + j_src] * kernel[k * ws.w_kernel + l];
                        }
                    }
                    ws.dst[i * ws.w_dst + j] = temp;
                }
            }

            break;
        default:
            printf("Unrecognized convolution mode, possible modes are :\n");
            printf("   - LINEAR_FULL \n");
            printf("   - LINEAR_SAME \n");
            printf("   - LINEAR_VALID \n");
            printf("   - CIRCULAR_SAME \n");
            printf("   - CIRCULAR_FULL \n");
            break;
        }
    }
}
