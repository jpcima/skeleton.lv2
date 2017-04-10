#pragma once
#include <memory>
#include <cstdint>

class Effect {
 public:
  explicit Effect(double rate);
  ~Effect();

  //============================================================================
  void connect_port(uint32_t port, void *data);

  //============================================================================
  void activate();
  void deactivate();

  //============================================================================
  void run(unsigned nframes);

 private:
  struct Impl;
  const std::unique_ptr<Impl> P;
};
