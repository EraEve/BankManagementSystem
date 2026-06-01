# Makefile - 银行智能管理系统
# 数据结构与算法课程设计 · 第十组 · Final v2.0

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
TARGET = bank_system
SRC = main.cpp
HEADERS = common.h employee_manager.h customer_manager.h card_manager.h \
          transaction_manager.h query_manager.h queue_manager.h \
          branch_manager.h smart_manager.h ui_manager.h

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

run: $(TARGET)
	./$(TARGET)

.PHONY: clean run
