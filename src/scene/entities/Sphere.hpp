#pragma once

#include "../Entity.hpp"

class Sphere : public Entity {
  
  public:
    Sphere();

  private:
    void load_mesh() override;

};
