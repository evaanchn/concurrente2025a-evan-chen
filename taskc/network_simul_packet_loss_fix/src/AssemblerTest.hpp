// Copyright 2020-2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#pragma once  // Directive that tells compiler to only set once

#include "Assembler.hpp"
#include "NetworkMessage.hpp"

/**
 * @brief A AssemblerTest class example
 */
class AssemblerTest : public Assembler<NetworkMessage, NetworkMessage> {
  DISABLE_COPY(AssemblerTest);  // Disablesd default methods for copying objects

 protected:
  /// Delay of consumer to consume a package, negative for max random
  const int consumerDelay = 0;

  /// Probability of packet loss
  const double packetLossProbability = 0.0;

  const size_t consumerCount = 0;

  // Number of messages lost
  size_t lostMessages = 0;

  /// Number of consecutive stop conditions received. Stop when two are received
  size_t stopConditionCount = 0;

 public:
  /// Constructor
  explicit AssemblerTest(const int consumerDelay
    , const double packetLossProbability, const size_t consumerCount);

  /// Consume the messages in its own execution thread
  int run() override;
  /// Override this method to process any data extracted from the queue
  void consume(NetworkMessage data) override;
};
