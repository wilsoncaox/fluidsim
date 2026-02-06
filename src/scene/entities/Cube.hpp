
#pragma once

#include "../Entity.hpp"

class Cube : public Entity {
  
  public:
    Cube();

  private:
    void load_mesh() override;

};
