#pragma once

#include "trade_data.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>

namespace financial {

/**
 * @brief 基础统计计算器
 */
class BaseStatsCalculator {
public:
    virtual ~BaseStatsCalculator() = default;
    
    /**
     * @brief 更新交易品种统计信息
     */
    virtual void updateStats(SymbolStats& stats, const Trade& trade) = 0;
    
    /**
     * @brief 完成统计计算（用于处理时间间隔等需要完整数据的计算）
     */
    virtual void finalizeStats(SymbolStats& stats) {}
};

/**
 * @brief 标准统计计算器
 * 计算最大时间间隔、总交易量、加权平均价格、最高价格
 */
class StandardStatsCalculator : public BaseStatsCalculator {
public:
    void updateStats(SymbolStats& stats, const Trade& trade) override;
    void finalizeStats(SymbolStats& stats) override;
    
private:
    struct SymbolData {
        uint64_t lastTimestamp = 0;
        uint64_t totalPriceQuantity = 0;  // 价格 * 数量的总和
        bool hasTrades = false;
    };
    
    std::unordered_map<std::string, SymbolData> symbolData_;
};

/**
 * @brief 扩展统计计算器（示例）
 * 可以添加更多统计指标
 */
class ExtendedStatsCalculator : public StandardStatsCalculator {
public:
    void updateStats(SymbolStats& stats, const Trade& trade) override;
    
private:
    std::unordered_map<std::string, uint32_t> minPrices_;
    std::unordered_map<std::string, uint64_t> tradeCounts_;
};

/**
 * @brief 交易处理器模板类
 * 使用模板设计，可以扩展不同的统计计算器
 */
template<typename StatsCalculator>
class TradeProcessor {
public:
    explicit TradeProcessor(std::unique_ptr<ITradeDataReader> reader,
                           std::unique_ptr<ITradeDataWriter> writer)
        : reader_(std::move(reader)), writer_(std::move(writer)) {}
    
    /**
     * @brief 处理交易数据并生成统计结果
     * @return 是否成功处理
     */
    bool process() {
        if (!reader_ || !writer_) {
            return false;
        }
        
        std::unordered_map<std::string, SymbolStats> symbolStatsMap;
        Trade trade;
        
        // 读取所有交易数据
        while (reader_->readNext(trade)) {
            auto& stats = symbolStatsMap[trade.symbol];
            if (stats.symbol.empty()) {
                stats.symbol = trade.symbol;
            }
            
            // 使用模板化的计算器更新统计信息
            calculator_.updateStats(stats, trade);
        }
        
        // 完成统计计算
        for (auto& [symbol, stats] : symbolStatsMap) {
            calculator_.finalizeStats(stats);
        }
        
        // 转换为向量并排序
        std::vector<SymbolStats> stats;
        stats.reserve(symbolStatsMap.size());
        for (auto& [symbol, stat] : symbolStatsMap) {
            stats.push_back(std::move(stat));
        }
        
        // 按交易品种排序
        std::sort(stats.begin(), stats.end(), 
                 [](const SymbolStats& a, const SymbolStats& b) {
                     return a.symbol < b.symbol;
                 });
        
        // 写入结果
        return writer_->writeStats(stats);
    }
    
    /**
     * @brief 获取计算器实例（用于测试）
     */
    const StatsCalculator& getCalculator() const { return calculator_; }
    
private:
    std::unique_ptr<ITradeDataReader> reader_;
    std::unique_ptr<ITradeDataWriter> writer_;
    StatsCalculator calculator_;
};

} // namespace financial 