//
//  StageUserData.cpp
//  crazy_snow
//
//  Created by qrh on 15-9-26.
//
//

#include "StageData.h"
#include "UserInfo.h"
#include "graph/GraphCommDef.h"
StageUserData::StageUserData()
:m_dirtyMask(0)
,m_notifyLock(0)
,m_curScore(0)
,m_totalNeedScore(0)
{}
void StageUserData::init(const StageBaseData& baseData, const UserInfo& userInfo) {
    m_curScore = userInfo.getCurScore();
    m_totalNeedScore = userInfo.getTotalNeedScore();
    next(baseData);
}
void StageUserData::next(const StageBaseData& baseData) {
    m_bonusNeedStar = baseData.m_bonusNeedStar;
    m_needScore = baseData.m_lbNeedScore;
    clear();
}
void StageUserData::clear() {
    m_lastBonusStar = m_bonusCurStar = 0;
    m_curBonusType = 0; m_nxtBonusType = randNodeSpec();
    
    m_stageClearFlag = false;
    
    m_dirtyMask = m_notifyLock = 0;
    m_observers.clear();
}

StageGameStatusVisitor StageUserData::lock() {
    return StageGameStatusVisitor(this);
}
void StageUserData::addObserver(void* privData, const StageUserDataObserver& observer) {
    m_observers.insert(std::make_pair(privData, observer));
}
void StageUserData::removeObserver(void* privData) {
    m_observers.erase(privData);
}
void StageUserData::notifyObservers() {
    if (m_dirtyMask != 0) {
        for (auto iter : m_observers) {
            iter.second(this);
        }
        m_dirtyMask = 0;
    }
}

void StageUserData::addBonusStar(int num) {
    m_lastBonusStar = num;
    m_bonusCurStar += num;
    if (m_bonusCurStar >= m_bonusNeedStar) {
        m_bonusCurStar -= m_bonusNeedStar;
        setCurBonusType(randNodeSpec());
    }
    m_dirtyMask |= DIRTY_BIT_BONUS_STAR;
}
void StageUserData::setCurBonusType(int newBonusType) {
    m_curBonusType = m_nxtBonusType;
    m_nxtBonusType = newBonusType;
    m_dirtyMask |= DIRTY_BIT_BONUS_TYPE;
}
void StageUserData::addCurScore(int score) {
    m_curScore += score;
    if (m_curScore >= m_totalNeedScore + m_needScore && ! m_stageClearFlag) {
        setStageClearFlag(true);
    }
    m_dirtyMask |= DIRTY_BIT_CURSCORE;
}
void StageUserData::setTotalNeedScore(int score) {
    m_totalNeedScore = score;
}
void StageUserData::setStageClearFlag(bool cleared) {
    if (! m_stageClearFlag && cleared) {
        m_totalNeedScore += m_needScore;
        m_needScore = 0;
    }
    m_stageClearFlag = cleared;
    m_dirtyMask |= DIRTY_BIT_STAGE_CLEAR_FLAG;
}


StageGameStatusVisitor::StageGameStatusVisitor(StageUserData* target)
:m_target(target) {
    m_target->m_notifyLock++;
}

StageGameStatusVisitor::~StageGameStatusVisitor() {
    if (--m_target->m_notifyLock == 0) {
        m_target->notifyObservers();
    }
}

int randNodeSpec() {
	static int s_seed = 0;
	if (s_seed == 0){
		s_seed = time(NULL);
		srand(s_seed);
	}
    
	if (rand()%6 >= 4) {
		static enBeadCategory seeds_spec[4] = {CHESS_CATEGORY_QUEEN, CHESS_CATEGORY_ARMOR, CHESS_CATEGORY_HORSE, CHESS_CATEGORY_SDOLIER};
		return seeds_spec[rand()%4];
	}
	else {
		static enBeadCategory seeds_spec[4] = {CHESS_CATEGORY_ELEPH, CHESS_CATEGORY_ARTILLERY, CHESS_CATEGORY_ARTILLERY, CHESS_CATEGORY_ELEPH};
		return seeds_spec[rand()%4];
	}
}