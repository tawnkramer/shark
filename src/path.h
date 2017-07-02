// path.h
//
// Author : Tawn Kramer
// July 2, 2017

#ifndef __PATH_H__
#define __PATH_H__

#include <vector>
#include "tmath.h"

struct PathNode
{
    TMath::Vector2 pos;
};

class Path
{
  public:

    Path()
    {
        m_looping = 0;
        m_iActiveSpan = 0;
    }

    void Reset();

    PathNode *GetActiveNode();

    void AddNode(const PathNode &n);

    void Start(const TMath::Vector2 &pos);

    // take a pos and update the cross track error.
    // returns true if still on the path, if non looping
    bool Update(const TMath::Vector2 &pos, float &crossTrackErr);

    bool m_looping;
    unsigned int m_iActiveSpan;
    std::vector<PathNode> m_nodes;
};

class PIDController
{
    public:

    PIDController();

    void SetVals(float p, float i, float d);

    void SetPath(Path* pPath) { m_pPath = pPath; }

    void Start(const TMath::Vector2 &pos);

    void Update(const TMath::Vector2& carPos, float& steering, float& throttle);

    Path* m_pPath;

    float Kp;
    float Kd;
    float Ki;
    float m_prevErr;
    float m_diffErr;
    float m_totalError;
};

#endif //__PATH_H__
