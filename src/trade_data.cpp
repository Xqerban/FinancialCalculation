#include "trade_data.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace financial {

// CsvTradeDataReader 实现
CsvTradeDataReader::CsvTradeDataReader(const std::string& filename) 
    : filename_(filename) {
    file_ = fopen(filename.c_str(), "r");
    if (!file_) {
        std::cerr << "无法打开输入文件: " << filename << std::endl;
    }
}

CsvTradeDataReader::~CsvTradeDataReader() {
    if (file_) {
        fclose(file_);
    }
}

bool CsvTradeDataReader::readNext(Trade& trade) {
    if (!file_) {
        return false;
    }
    char line[1024];
    while (fgets(line, sizeof(line), file_)) {
        // 移除换行符
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        std::string strLine(line);
        // 跳过空行
        if (strLine.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }
        if (parseLine(strLine, trade)) {
            return true;
        } else {
            // std::cerr << "解析失败: " << strLine << std::endl;
            continue;
        }
    }
    return false;
}

void CsvTradeDataReader::reset() {
    if (file_) {
        rewind(file_);
    }
}

bool CsvTradeDataReader::parseLine(const std::string& line, Trade& trade) {
    std::istringstream iss(line);
    std::string token;
    
    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    };
    
    // 解析时间戳
    if (!std::getline(iss, token, ',')) {
        return false;
    }
    trim(token);
    try {
        trade.timestamp = std::stoull(token);
    } catch (const std::exception&) {
        return false;
    }
    
    // 解析交易品种
    if (!std::getline(iss, trade.symbol, ',')) {
        return false;
    }
    trim(trade.symbol);
    
    // 解析数量
    if (!std::getline(iss, token, ',')) {
        return false;
    }
    trim(token);
    try {
        trade.quantity = std::stoul(token);
    } catch (const std::exception&) {
        return false;
    }
    
    // 解析价格
    if (!std::getline(iss, token, ',')) {
        return false;
    }
    trim(token);
    try {
        trade.price = std::stoul(token);
    } catch (const std::exception&) {
        return false;
    }
    
    return true;
}

// CsvTradeDataWriter 实现
CsvTradeDataWriter::CsvTradeDataWriter(const std::string& filename) 
    : filename_(filename) {
    file_ = fopen(filename.c_str(), "w");
    if (!file_) {
        std::cerr << "无法创建输出文件: " << filename << std::endl;
    }
}

CsvTradeDataWriter::~CsvTradeDataWriter() {
    if (file_) {
        fclose(file_);
    }
}

bool CsvTradeDataWriter::writeStats(const std::vector<SymbolStats>& stats) {
    if (!file_) {
        return false;
    }
    
    // 写入标题行
    fprintf(file_, "symbol,MaxTimeGap,Volume,WeightedAveragePrice,MaxPrice\n");
    
    // 写入数据行
    for (const auto& stat : stats) {
        fprintf(file_, "%s,%llu,%llu,%u,%u\n",
                stat.symbol.c_str(),
                stat.maxTimeGap,
                stat.totalVolume,
                stat.weightedAvgPrice,
                stat.maxPrice);
    }
    
    return true;
}

} // namespace financial 