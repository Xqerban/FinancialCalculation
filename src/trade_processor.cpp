#include "trade_processor.h"
#include <algorithm>
#include <iostream>

namespace financial {

void StandardStatsCalculator::updateStats(SymbolStats& stats, const Trade& trade) {
    // 更新总交易量
    stats.totalVolume += trade.quantity;
    
    // 更新最高价格
    if (trade.price > stats.maxPrice) {
        stats.maxPrice = trade.price;
    }
    
    // 更新加权平均价格计算
    stats.weightedAvgPrice = 0;  // 临时设为0，在finalizeStats中计算
    
    // 处理时间间隔
    auto& symbolData = symbolData_[trade.symbol];
    if (symbolData.hasTrades) {
        uint64_t timeGap = trade.timestamp - symbolData.lastTimestamp;
        if (timeGap > stats.maxTimeGap) {
            stats.maxTimeGap = timeGap;
        }
    }
    
    // 更新内部数据
    symbolData.lastTimestamp = trade.timestamp;
    symbolData.totalPriceQuantity += static_cast<uint64_t>(trade.price) * trade.quantity;
    symbolData.hasTrades = true;
}

void StandardStatsCalculator::finalizeStats(SymbolStats& stats) {
    auto it = symbolData_.find(stats.symbol);
    if (it != symbolData_.end()) {
        const auto& symbolData = it->second;
        if (stats.totalVolume > 0) {
            // 计算加权平均价格：总价格*数量 / 总数量
            stats.weightedAvgPrice = static_cast<uint32_t>(
                symbolData.totalPriceQuantity / stats.totalVolume);
        }
    }
}

void ExtendedStatsCalculator::updateStats(SymbolStats& stats, const Trade& trade) {
    // 调用基类的更新方法
    StandardStatsCalculator::updateStats(stats, trade);
    
    // 更新最低价格
    auto& minPrice = minPrices_[trade.symbol];
    if (minPrice == 0 || trade.price < minPrice) {
        minPrice = trade.price;
    }
    
    // 更新交易次数
    tradeCounts_[trade.symbol]++;
}

} // namespace financial 