#include "trade_processor.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace financial;

/**
 * @brief 内存中的测试数据读取器
 */
class TestTradeDataReader : public ITradeDataReader {
public:
    explicit TestTradeDataReader(const std::string& data) : data_(data) {
        reset();
    }
    
    bool readNext(Trade& trade) override {
        std::string line;
        if (std::getline(stream_, line)) {
            return parseLine(line, trade);
        }
        return false;
    }
    
    void reset() override {
        stream_.clear();
        stream_.str(data_);
    }
    
private:
    std::string data_;
    std::istringstream stream_;
    
    bool parseLine(const std::string& line, Trade& trade) {
        std::istringstream iss(line);
        std::string token;
        
        // 解析时间戳
        if (!std::getline(iss, token, ',')) return false;
        trade.timestamp = std::stoull(token);
        
        // 解析交易品种
        if (!std::getline(iss, trade.symbol, ',')) return false;
        
        // 解析数量
        if (!std::getline(iss, token, ',')) return false;
        trade.quantity = std::stoul(token);
        
        // 解析价格
        if (!std::getline(iss, token, ',')) return false;
        trade.price = std::stoul(token);
        
        return true;
    }
};

/**
 * @brief 内存中的测试数据写入器
 */
class TestTradeDataWriter : public ITradeDataWriter {
public:
    bool writeStats(const std::vector<SymbolStats>& stats) override {
        output_.clear();
        for (const auto& stat : stats) {
            output_ += stat.symbol + "," + 
                      std::to_string(stat.maxTimeGap) + "," +
                      std::to_string(stat.totalVolume) + "," +
                      std::to_string(stat.weightedAvgPrice) + "," +
                      std::to_string(stat.maxPrice) + "\n";
        }
        return true;
    }
    
    const std::string& getOutput() const { return output_; }
    
private:
    std::string output_;
};

void testBasicFunctionality() {
    std::cout << "Testing basic functionality..." << std::endl;
    
    // 测试数据
    std::string testData = 
        "52924702,aaa,13,1136\n"
        "52924702,aac,20,477\n"
        "52925641,aab,31,907\n"
        "52927350,aab,29,724\n"
        "52927783,aac,21,638\n"
        "52930489,aaa,18,1222\n"
        "52931654,aaa,9,1077\n"
        "52933453,aab,9,756\n";
    
    auto reader = std::make_unique<TestTradeDataReader>(testData);
    auto writer = std::make_unique<TestTradeDataWriter>();
    auto writer_ptr = writer.get();
    
    TradeProcessor<StandardStatsCalculator> processor(std::move(reader), std::move(writer));
    
    bool success = processor.process();
    assert(success);
    
    // 验证输出结果
    const std::string& output = writer_ptr->getOutput();
    std::cout << "Output: " << output << std::endl;
    
    // 检查是否包含预期的交易品种
    assert(output.find("aaa") != std::string::npos && "Should contain aaa");
    assert(output.find("aab") != std::string::npos && "Should contain aab");
    assert(output.find("aac") != std::string::npos && "Should contain aac");
    
    std::cout << "Basic functionality test passed" << std::endl;
}

void testSingleTrade() {
    std::cout << "Testing single trade..." << std::endl;
    
    std::string testData = "1234567,aaa,10,12\n";
    
    auto reader = std::make_unique<TestTradeDataReader>(testData);
    auto writer = std::make_unique<TestTradeDataWriter>();
    auto writer_ptr = writer.get();
    
    TradeProcessor<StandardStatsCalculator> processor(std::move(reader), std::move(writer));
    
    bool success = processor.process();
    assert(success);
    
    // 验证单笔交易的结果
    const std::string& output = writer_ptr->getOutput();
    std::cout << "Output: " << output << std::endl;
    
    // 检查是否包含预期的结果
    assert(output.find("aaa,0,10,12,12") != std::string::npos);
    
    std::cout << "Single trade test passed" << std::endl;
}

void testWeightedAveragePrice() {
    std::cout << "Testing weighted average price..." << std::endl;
    
    // 测试数据：20股@18, 5股@7，加权平均价格应该是15
    // (20*18 + 5*7) / (20+5) = (360 + 35) / 25 = 395 / 25 = 15.8 -> 15 (truncated)
    std::string testData = 
        "1000000,aaa,20,18\n"
        "1000001,aaa,5,7\n";
    
    auto reader = std::make_unique<TestTradeDataReader>(testData);
    auto writer = std::make_unique<TestTradeDataWriter>();
    auto writer_ptr = writer.get();
    
    TradeProcessor<StandardStatsCalculator> processor(std::move(reader), std::move(writer));
    
    bool success = processor.process();
    assert(success);
    
    // 验证加权平均价格计算
    const std::string& output = writer_ptr->getOutput();
    std::cout << "Output: " << output << std::endl;
    
    // 检查加权平均价格是否为15
    assert(output.find("aaa,1,25,15,18") != std::string::npos);
    
    std::cout << "Weighted average price test passed" << std::endl;
}

void testTimeGap() {
    std::cout << "Testing time gap..." << std::endl;
    
    // 测试数据：时间间隔计算
    // 1000000 -> 1000500 -> 1001000
    // 最大间隔应该是 1001000 - 1000000 = 1000
    std::string testData = 
        "1000000,aaa,10,100\n"
        "1000500,aaa,10,150\n"
        "1001000,aaa,10,200\n";
    
    auto reader = std::make_unique<TestTradeDataReader>(testData);
    auto writer = std::make_unique<TestTradeDataWriter>();
    auto writer_ptr = writer.get();
    
    TradeProcessor<StandardStatsCalculator> processor(std::move(reader), std::move(writer));
    
    bool success = processor.process();
    assert(success);
    
    // 验证时间间隔计算
    const std::string& output = writer_ptr->getOutput();
    std::cout << "Output: " << output << std::endl;
    
    // 检查最大时间间隔是否为500
    assert(output.find("aaa,500,30,") != std::string::npos);
    
    std::cout << "Time gap test passed" << std::endl;
}

void testExtendedCalculator() {
    std::cout << "Testing extended calculator..." << std::endl;
    
    std::string testData = 
        "1000000,aaa,10,100\n"
        "1001000,aaa,10,200\n";
    
    auto reader = std::make_unique<TestTradeDataReader>(testData);
    auto writer = std::make_unique<TestTradeDataWriter>();
    auto writer_ptr = writer.get();
    
    TradeProcessor<ExtendedStatsCalculator> processor(std::move(reader), std::move(writer));
    
    bool success = processor.process();
    assert(success);
    
    // 验证扩展计算器的结果
    const std::string& output = writer_ptr->getOutput();
    std::cout << "Output: " << output << std::endl;
    
    // 检查基本统计是否正确
    assert(output.find("aaa,1000,20,150,200") != std::string::npos);
    
    std::cout << "Extended calculator test passed" << std::endl;
}

int main() {
    std::cout << "Starting unit tests..." << std::endl;
    
    try {
        testBasicFunctionality();
        testSingleTrade();
        testWeightedAveragePrice();
        testTimeGap();
        testExtendedCalculator();
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
} 