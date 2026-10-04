#pragma once

#include "tgaimage.h"

class Rasterizer {
    explicit Rasterizer() : m_framebuffer() {}

   private:
    TGAImage m_framebuffer;

    virtual void triangle();
};
