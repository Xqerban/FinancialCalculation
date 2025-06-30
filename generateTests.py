#!/usr/bin/env python3
"""
大数据生成器 - 用于测试程序的内存效率
生成超过物理内存大小的测试数据
"""

import random
import time
import string

def generate_large_dataset(filename, target_size_gb=2):
    """
    生成大型测试数据集
    
    Args:
        filename: 输出文件名
        target_size_gb: 目标文件大小（GB）
    """
    print(f"Generating {target_size_gb}GB test dataset...")
    
    # 交易品种列表
    symbols = [''.join(random.choices(string.ascii_uppercase, k=3)) for _ in range(17575)]
    
    # 时间戳基准
    base_timestamp = 52924702
    
    target_size_bytes = target_size_gb * 1024 * 1024 * 1024
    current_size = 0
    line_count = 0
    
    start_time = time.time()
    
    with open(filename, 'w') as f:
        while current_size < target_size_bytes:
            # 生成随机交易数据
            timestamp = base_timestamp + random.randint(0, 1000000)
            symbol = random.choice(symbols)
            quantity = random.randint(1, 1000)
            price = random.randint(10, 10000)
            
            # 写入CSV行
            line = f"{timestamp},{symbol},{quantity},{price}\n"
            f.write(line)
            
            current_size += len(line.encode('utf-8'))
            line_count += 1
            
            # 每100万行显示进度
            if line_count % 1000000 == 0:
                elapsed = time.time() - start_time
                size_mb = current_size / (1024 * 1024)
                print(f"Generated {line_count:,} lines, {size_mb:.1f}MB, "
                      f"elapsed: {elapsed:.1f}s")
    
    elapsed = time.time() - start_time
    size_gb = current_size / (1024 * 1024 * 1024)
    print(f"Generation completed!")
    print(f"Total lines: {line_count:,}")
    print(f"File size: {size_gb:.2f}GB")
    print(f"Time elapsed: {elapsed:.1f}s")
    print(f"Generation speed: {size_gb/elapsed:.2f}GB/s")

def generate_memory_test():
    """生成内存测试数据集（较小，用于快速测试）"""
    print("Generating memory test dataset (100MB)...")
    generate_large_dataset("memory_test.csv", 0.1)  # 100MB

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1:
        size_gb = float(sys.argv[1])
        generate_large_dataset("input.csv", size_gb)
    else:
        generate_memory_test()