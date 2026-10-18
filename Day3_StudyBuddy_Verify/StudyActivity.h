#pragma once

enum StudyState { ST_UNSURE, ST_AWAY, ST_PRESENT, ST_STUDYING, ST_COMPUTER, ST_PHONE };

// 手機先暫停獎勵；書和電腦同時看到，就先陪讀。
constexpr StudyState classifyActivity(bool phone, bool book, bool computer) {
    return phone ? ST_PHONE : book ? ST_STUDYING : computer ? ST_COMPUTER : ST_PRESENT;
}
constexpr bool isSeatedState(StudyState s) {
    return s == ST_PRESENT || s == ST_STUDYING || s == ST_COMPUTER || s == ST_PHONE;
}
constexpr bool earnsLearningReward(StudyState s) {
    return s == ST_STUDYING || s == ST_COMPUTER;
}
