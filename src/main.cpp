#include "trade_data.h"
#include "trade_processor.h"
#include <iostream>
#include <memory>

using namespace financial;

int main(int argc, char* argv[]) {
    const std::string inputFile = "input.csv";
    const std::string outputFile = "output.csv";
    
    std::cout << "Financial Trade Data Processor" << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    
    try {
        // Create data reader and writer
        auto reader = std::make_unique<CsvTradeDataReader>(inputFile);
        auto writer = std::make_unique<CsvTradeDataWriter>(outputFile);
        
        // Create trade processor (using standard stats calculator)
        TradeProcessor<StandardStatsCalculator> processor(std::move(reader), std::move(writer));
        
        // Process data
        if (processor.process()) {
            std::cout << "Data processing completed! Result saved to " << outputFile << std::endl;
            return 0;
        } else {
            std::cerr << "Data processing failed!" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 