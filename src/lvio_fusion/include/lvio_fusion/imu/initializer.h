#ifndef lvio_fusion_INITIALIZER_H
#define lvio_fusion_INITIALIZER_H

#include "lvio_fusion/common.h"
#include "lvio_fusion/imu/imu.h"
#include "lvio_fusion/map.h"

namespace lvio_fusion
{

class Initializer
{
public:
    typedef std::shared_ptr<Initializer> Ptr;

    void Initialize(double init_time, double end_time);

    int step = 1;   // 1,2,3: next step 1,2,3; 4: finish;

private:
    void EstimateVelAndRwg(Frames keyframes);

    bool Initialize(Frames frames, double prior_a, double prior_g);

    Matrix3d Rwg_;  // R of gravity in world frame
    const int num_frames_init = 10;
};

} // namespace lvio_fusion
#endif // lvio_fusion_INITIALIZER_H
