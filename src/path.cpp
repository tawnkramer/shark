#include <math.h>
#include <stdlib.h>
#include "path.h"
#include "tmath.h"

using namespace TMath;

void Path::Reset()
{
    m_iActiveSpan = 0;
    m_nodes.clear();
}

void Path::AddNode(const PathNode &n)
{
    m_nodes.push_back(n);
}

PathNode *Path::GetActiveNode()
{
    if (m_iActiveSpan < m_nodes.size())
        return &m_nodes[m_iActiveSpan];

    return NULL;
}

void Path::Start(const TMath::Vector2 &pos)
{
    int iClosest = -1;
    float closestDist = 1000000.0f;

    for(int iN = 0; iN < m_nodes.size(); iN++)
    {
        PathNode& n = m_nodes[iN];
        float d = (n.pos - pos).Mag();

        if(d < closestDist)
        {
            d = closestDist;
            iClosest = iN;
            m_iActiveSpan = iN;
        }
    }
}

// take a pos and update the cross track error in err.
// returns true if still on the path, if non looping

bool Path::Update(const TMath::Vector2& pos, float& crossTrackErr)
{
    if(m_iActiveSpan >= m_nodes.size() - 2)
    {
        if(!m_looping)
            return false;

        m_iActiveSpan = 0;
    }

    PathNode& a = m_nodes[m_iActiveSpan];
    PathNode& b = m_nodes[m_iActiveSpan + 1];

    LineSeg2d pathSeg(a.pos, b.pos);

	LineSeg2d::LineSegResult offEnd;

    Vector2 closePt = pathSeg.ClosestPointOnLineTo(pos, offEnd);

    if(offEnd == LineSeg2d::eOffEndB)
    {
        m_iActiveSpan++;
    }
    else if(offEnd == LineSeg2d::eOffEndA)
    {
        if(m_iActiveSpan > 0)
            m_iActiveSpan--;
    }

    Vector2 errVec = pathSeg.ClosestVectorTo( pos, offEnd );

    float sign = 1.0f;

    float mag = errVec.Normalize();

    float angle = acosf(errVec.Dot(pathSeg.m_ray));

    if(angle < 0.0f)
        sign = -1.0f;

    crossTrackErr = mag * sign;
    
    return true;
}

PIDController::PIDController()
{
    Kp = 10.0f;
    Kd = 10.0f;
    Ki = 0.1f;
    m_pPath = NULL;
    m_prevErr = 0.0f;
    m_diffErr = 0.0f;
    m_totalError = 0.0f;
}

void PIDController::SetVals(float p, float i, float d)
{
    Kp = p;
    Ki = i;
    Kd = d;
}

void PIDController::Start(const TMath::Vector2 &pos)
{
    m_prevErr = 0.0f;
    m_diffErr = 0.0f;
    m_totalError = 0.0f;

    if(m_pPath != NULL)
        m_pPath->Start(pos);

}

void PIDController::Update(const Vector2& pos, float& steering, float& throttle)
{
    if(m_pPath == NULL)
        return;

    float crossTrackErr = 0.0f;

    if(m_pPath->Update(pos, crossTrackErr))
    {
        m_diffErr = crossTrackErr - m_prevErr;

        steering = (-Kp * crossTrackErr) - (Kd * m_diffErr) - (Ki * m_totalError);

        throttle = 1.0f;

        //accumulate total error
		m_totalError += crossTrackErr;

		//save err for next iteration.
		m_prevErr = crossTrackErr;
    }
    else
    {
        throttle = 0.0f;
    }
}
