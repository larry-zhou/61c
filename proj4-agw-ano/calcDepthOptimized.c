// CS 61C Fall 2015 Project 4

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

// include OpenMP
#if !defined(_MSC_VER)
#include <pthread.h>
#endif
#include <omp.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

/* DO NOT CHANGE ANYTHING ABOVE THIS LINE. */
int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}
int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}
float displacementOptimized(int dx, int dy)
{
    float squaredDisplacement = dx * dx + dy * dy;
    float displacement = sqrt(squaredDisplacement);
    return displacement;
}

void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement) {
    memset(depth, 0, imageHeight * imageWidth * sizeof(float));
    #pragma omp parallel for
    for (int y = featureHeight; y < imageHeight - featureHeight; y++) {
        for (int x = featureWidth; x < imageWidth - featureWidth; x++)  {
            float minimumSquaredDifference = -1;
            int minimumDy = 0;
            int minimumDx = 0;
            for (int dx = max(featureWidth - x, -maximumDisplacement); dx <= min(maximumDisplacement, imageWidth - featureWidth - x - 1); dx++) {
                for (int dy = max(featureHeight - y, -maximumDisplacement); dy <= min(maximumDisplacement, imageHeight - featureHeight - y - 1); dy++) {

                    float squaredDifference = 0;
                    __m128 sum_vector = _mm_setzero_ps(), tail_case = _mm_setzero_ps();
                    __m128 left_vector, right_vector, sub_vector, mult_vector;
                    int edgeX = 0, boxX;
                    /* Sum the squared difference within a box of +/- featureHeight and +/- featureWidth. */
                    /* increment boxX by 8 because we are working on 8 X's*/
                    /* increment boxY normally */
                    /* only do so if our box width is bigger than 8 */
                    /* switch x and y for spatial locality */
                    if ((2 * featureWidth + 1) >= 8) {
                        for (boxX = -featureWidth; boxX <= featureWidth; boxX += 8) {
                            for (int boxY = -featureHeight; boxY <= featureHeight; boxY++) {
                                //if we would be outside the width, then break, and have "edge" case. lol
                                if (boxX + 8 > featureWidth) {
                                    edgeX = boxX;
                                    break;
                                }
                                /* do operations on four single precisions at once */
                                for(int t = 0; t <= 4; t += 4) {
                                    int leftX = x + boxX;
                                    int leftY = y + boxY;
                                    int rightX = leftX + dx;
                                    int rightY = leftY + dy;
                                    left_vector = _mm_loadu_ps((left + (leftY * imageWidth + leftX + t)));
                                    right_vector = _mm_loadu_ps((right + (rightY * imageWidth + rightX + t)));
                                    sub_vector = _mm_sub_ps(left_vector, right_vector);
                                    mult_vector = _mm_mul_ps(sub_vector, sub_vector);
                                    sum_vector = _mm_add_ps(sum_vector, mult_vector);
                                }
                            }
                        }
                        if (featureWidth >= edgeX + 4) { 
                            for (boxX = edgeX; boxX <= featureWidth; boxX += 4) {
                                for (int boxY = -featureHeight; boxY <= featureHeight; boxY++) {
                                    //again, if we going out of our feature box, then break
                                    if (boxX > featureWidth - 4) {
                                        edgeX = boxX;
                                        break;
                                    }
                                    //otherwise, do our routine
                                    int leftX = x + boxX;
                                    int leftY = y + boxY;
                                    int rightX = leftX + dx;
                                    int rightY = leftY + dy;
                                    left_vector = _mm_loadu_ps((left + (leftY *imageWidth + leftX)));
                                    right_vector = _mm_loadu_ps((right + (rightY *imageWidth + rightX)));
                                    sub_vector = _mm_sub_ps(left_vector, right_vector);
                                    mult_vector = _mm_mul_ps(sub_vector, sub_vector);
                                    sum_vector = _mm_add_ps(sum_vector, mult_vector);

                                } 
                            }
                        }
                    }
                    /* if its not big enough to do 8, only do the iteration once */
                    else {
                        for (boxX = -featureWidth; boxX <= featureWidth; boxX += 4) {
                            for (int boxY = -featureHeight; boxY <= featureHeight; boxY++) {
                                //making sure we are not out of bounds again
                                if (boxX + 4 > featureWidth) {
                                    edgeX = boxX;
                                    break;
                                }

                                int leftX = x + boxX;
                                int leftY = y + boxY;
                                int rightX = leftX + dx;
                                int rightY = leftY + dy;
                                left_vector = _mm_loadu_ps((left + (leftY * imageWidth + leftX)));
                                right_vector = _mm_loadu_ps((right + (rightY *imageWidth + rightX)));
                                sub_vector = _mm_sub_ps(left_vector, right_vector);
                                mult_vector = _mm_mul_ps(sub_vector, sub_vector);
                                sum_vector = _mm_add_ps(sum_vector, mult_vector);


                            }
                        }
                    }
                    float A[4] = {0, 0, 0, 0};
                    _mm_storeu_ps(A, sum_vector);
                    squaredDifference += (A[0] + A[1] + A[2] + A[3]);
                    //taken from lab 9
                    
                    //if squaredDifference isn't smaller before the tail_case, then we should move on.
                    if (squaredDifference >= minimumSquaredDifference && minimumSquaredDifference != -1) 
                        continue;

                    //Even and Odd Case
                    if (featureWidth % 2 == 0) {
                        for (int boxY = -featureHeight; boxY <= featureHeight; boxY++) {
                            int leftX = x + edgeX;
                            int leftY = y + boxY;
                            int rightX = leftX + dx;
                            int rightY = leftY + dy;
                            float difference = left[leftY * imageWidth + leftX] - right[rightY * imageWidth + rightX];
                            squaredDifference += difference * difference;
                        }
                    }
           
                    else  {
                        for (int boxY = -featureHeight; boxY <= featureHeight; boxY++) {
                            int leftX = x + edgeX;
                            int leftY = y + boxY;
                            int rightX = leftX + dx;
                            int rightY = leftY + dy;
                            left_vector = _mm_loadu_ps((left + (leftY * imageWidth + leftX)));
                            right_vector = _mm_loadu_ps((right + (rightY *imageWidth + rightX)));
                            sub_vector = _mm_sub_ps(left_vector, right_vector);
                            mult_vector = _mm_mul_ps(sub_vector, sub_vector);
                            tail_case = _mm_add_ps(tail_case, mult_vector);
                        }
                        //same as lab 9
                        _mm_storeu_ps(A, tail_case);
                        squaredDifference += (A[0] + A[1] + A[2]);
                    }

                    /* 
                    Check if you need to update minimum square difference. 
                    This is when either it has not been set yet, the current
                    squared displacement is equal to the min and but the new
                    displacement is less, or the current squared difference
                    is less than the min square difference.
                    */
                    if ((minimumSquaredDifference == -1) || ((minimumSquaredDifference == squaredDifference) && (displacementNaive(dx, dy) < displacementNaive(minimumDx, minimumDy))) || (minimumSquaredDifference > squaredDifference))
                    {
                        minimumSquaredDifference = squaredDifference;
                        minimumDx = dx;
                        minimumDy = dy;
                    }
                }
            }
            /* 
            Set the value in the depth map. 
            If max displacement is equal to 0, the depth value is just 0.
            */
            if (minimumSquaredDifference != -1) {
                if (maximumDisplacement == 0) {
                    depth[y * imageWidth + x] = 0;
                } else {
                    depth[y * imageWidth + x] = displacementNaive(minimumDx, minimumDy);
                }
            } else {
                depth[y * imageWidth + x] = 0;
            }
        }
    }
}