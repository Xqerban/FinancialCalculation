#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace financial {

/**
 * @brief 表示单个交易记录
 */
struct Trade {
    uint64_t timestamp;  // 微秒时间戳
    std::string symbol;  // 3字符交易品种标识
    uint32_t quantity;   // 交易数量
    uint32_t price;      // 交易价格
    
    Trade() = default;
    Trade(uint64_t ts, std::string sym, uint32_t qty, uint32_t prc)
        : timestamp(ts), symbol(std::move(sym)), quantity(qty), price(prc) {}
};

/**
 * @brief 表示交易品种的统计结果
 */
struct SymbolStats {
    std::string symbol;
    uint64_t maxTimeGap = 0;        // 最大时间间隔
    uint64_t totalVolume = 0;       // 总交易量
    uint32_t weightedAvgPrice = 0;  // 加权平均价格
    uint32_t maxPrice = 0;          // 最高价格
    
    SymbolStats() = default;
    explicit SymbolStats(std::string sym) : symbol(std::move(sym)) {}
};

/**
 * @brief 交易数据读取器接口
 */
class ITradeDataReader {
public:
    virtual ~ITradeDataReader() = default;
    virtual bool readNext(Trade& trade) = 0;
    virtual void reset() = 0;
};

/**
 * @brief CSV文件交易数据读取器
 */
class CsvTradeDataReader : public ITradeDataReader {
public:
    explicit CsvTradeDataReader(const std::string& filename);
    ~CsvTradeDataReader() override;
    
    bool readNext(Trade& trade) override;
    void reset() override;
    
private:
    std::string filename_;
    FILE* file_ = nullptr;
    bool parseLine(const std::string& line, Trade& trade);
};

/**
 * @brief 交易数据写入器接口
 */
class ITradeDataWriter {
public:
    virtual ~ITradeDataWriter() = default;
    virtual bool writeStats(const std::vector<SymbolStats>& stats) = 0;
};

/**
 * @brief CSV文件交易数据写入器
 */
class CsvTradeDataWriter : public ITradeDataWriter {
public:
    explicit CsvTradeDataWriter(const std::string& filename);
    ~CsvTradeDataWriter() override;
    
    bool writeStats(const std::vector<SymbolStats>& stats) override;
    
private:
    std::string filename_;
    FILE* file_ = nullptr;
};

} // namespace financial 