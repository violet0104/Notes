#include "Buffer.h"
#include <cassert>
#include <iostream>
#include <string>

void TestAppendAndSize() {
  Buffer buf;
  buf.Append("hello", 5);
  assert(buf.Size() == 5);
  assert(buf.HasData());

  // Test embedded nulls (which used to break the old Buffer)
  char dataWithNull[] = {'a', '\0', 'b'};
  buf.Append(dataWithNull, 3);
  assert(buf.Size() == 8); // 5 + 3
  const char *raw_data = buf.ToStr();
  for (int i = 0; i < buf.Size(); ++i) {
    if (raw_data[i] == '\0') {
      std::cout << "\\0"; // 把看不见的空字符打印出来，方便人眼观察
    } else {
      std::cout << raw_data[i];
    }
  }
  std::cout << std::endl;
  std::cout << "TestAppendAndSize passed!" << std::endl;
}

void TestFindAndPopUntil_StickyPacket() {
  Buffer buf;
  // Simulate two packets arriving at once (Sticky Packet / 粘包)
  std::string p1 = "GET / HTTP/1.1\r\n";
  std::string p2 = "Host: localhost\r\n";
  buf.Append(p1.c_str(), p1.size());
  buf.Append(p2.c_str(), p2.size());

  auto res1 = buf.FindAndPopUntil("\r\n");
  assert(res1.first);
  assert(res1.second == "GET / HTTP/1.1");

  auto res2 = buf.FindAndPopUntil("\r\n");
  assert(res2.first);
  assert(res2.second == "Host: localhost");

  assert(!buf.HasData());
  std::cout << "TestFindAndPopUntil_StickyPacket passed!" << std::endl;
}

void TestFindAndPopUntil_HalfPacket() {
  Buffer buf;
  // Simulate half a packet arriving (Half Packet / 半包)
  std::string half = "GET / HTT";
  buf.Append(half.c_str(), half.size());

  auto res1 = buf.FindAndPopUntil("\r\n");
  assert(!res1.first); // Should not find the delimiter yet

  // Simulate the rest of the packet arriving later
  std::string rest = "P/1.1\r\n";
  buf.Append(rest.c_str(), rest.size());

  auto res2 = buf.FindAndPopUntil("\r\n");
  assert(res2.first);
  assert(res2.second == "GET / HTTP/1.1");
  assert(!buf.HasData());
  std::cout << "TestFindAndPopUntil_HalfPacket passed!" << std::endl;
}

void TestRetrieve() {
  Buffer buf;
  buf.Append("1234567890", 10);
  buf.Retrieve(3);
  assert(buf.Size() == 7);

  std::pair<bool, std::string> res = buf.FindAndPopUntil("0");
  assert(res.first);
  assert(res.second == "456789");

  std::cout << "TestRetrieve passed!" << std::endl;
}

int main() {
  TestAppendAndSize();
  TestFindAndPopUntil_StickyPacket();
  TestFindAndPopUntil_HalfPacket();
  TestRetrieve();
  std::cout << "All Buffer tests passed successfully!" << std::endl;
  return 0;
}
