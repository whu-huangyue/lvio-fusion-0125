#include "lvio_fusion/map.h"
#include "lvio_fusion/visual/feature.h"

namespace lvio_fusion
{

void Map::InsertKeyFrame(Frame::Ptr frame)
{
    std::unique_lock<std::mutex> lock(mutex_local_kfs);
    Frame::current_frame_id++;
    keyframes[frame->time] = frame;
}

void Map::InsertLandmark(visual::Landmark::Ptr landmark)
{
    std::unique_lock<std::mutex> lock(mutex_local_kfs);
    landmarks[landmark->id] = landmark;
}

// time < 0 or time > end: return the last one
Frame::Ptr Map::GetKeyFrame(double time)
{
    if (time < 0)
        return (--keyframes.end())->second;
    auto iter = keyframes.lower_bound(time);
    if (iter == keyframes.end())
    {
        return (--keyframes.end())->second;
    }
    else
    {
        auto last_iter = iter;
        last_iter--;
        if (iter == keyframes.begin() || time - last_iter->first > iter->first - time)
        {
            return iter->second;
        }
        else
        {
            return last_iter->second;
        }
    }
}

// 1: [start]
// 2: [start -> end]
// 3: (start -> num]
// 4: [num -> end)
Frames Map::GetKeyFrames(double start, double end, int num)
{
    if (end == 0 && num == 0)
    {
        auto start_iter = keyframes.lower_bound(start);
        return start_iter == keyframes.end() ? Frames() : Frames(start_iter, keyframes.end());
    }
    else if (num == 0)
    {
        auto start_iter = keyframes.lower_bound(start);
        auto end_iter = keyframes.upper_bound(end);
        return start > end ? Frames() : Frames(start_iter, end_iter);
    }
    else if (end == 0)
    {
        auto iter = keyframes.upper_bound(start);
        Frames frames;
        for (int i = 0; i < num && iter != keyframes.end(); i++)
        {
            frames.insert(*(iter++));
        }
        return frames;
    }
    else if (start == 0)
    {
        auto iter = keyframes.lower_bound(end);
        Frames frames;
        for (int i = 0; i < num && iter != keyframes.begin(); i++)
        {
            frames.insert(*(--iter));
        }
        return frames;
    }
    return Frames();
}

void Map::RemoveLandmark(visual::Landmark::Ptr landmark)
{
    std::unique_lock<std::mutex> lock(mutex_local_kfs);
    landmark->Clear();
    landmarks.erase(landmark->id);
}

SE3d Map::ComputePose(double time)
{
    auto frame1 = keyframes.lower_bound(time)->second;
    auto frame2 = keyframes.upper_bound(time)->second;
    double d_t = time - frame1->time;
    double t_t = frame2->time - frame1->time;
    double s = d_t / t_t;
    Quaterniond q = frame1->pose.unit_quaternion().slerp(s, frame2->pose.unit_quaternion());
    Vector3d t = (1 - s) * frame1->t() + s * frame2->t();
    return SE3d(q, t);
}

void Map::ApplyGravityRotation(const Matrix3d &R)
{
    Quaterniond q(R);
    for (auto pair : keyframes)
    {
        Frame::Ptr frame = pair.second;
        frame->SetPose(q * frame->R(), q * frame->t());
        frame->Vw = q * frame->Vw;
    }
}

} // namespace lvio_fusion
