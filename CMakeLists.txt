cmake_minimum_required(VERSION 3.10)
project(FinancialCalculation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试
enable_testing()

# 主程序
add_executable(financial_calc 
    src/main.cpp
    src/trade_processor.cpp
    src/trade_data.cpp
)

# 测试程序
add_executable(trade_processor_test 
    tests/test_trade_processor.cpp
    src/trade_processor.cpp
    src/trade_data.cpp
)

# 包含目录
target_include_directories(financial_calc PRIVATE include)
target_include_directories(trade_processor_test PRIVATE include)

# 添加测试
add_test(NAME TradeProcessorTest COMMAND trade_processor_test) 