#ifndef TEXTURE_H
#define TEXTURE_H

class texture {
public:
    unsigned int m_id;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_internalFormat;
    unsigned int m_imageFormat;
    unsigned int m_wrapS;
    unsigned int m_wrapT;
    unsigned int m_filterMin;
    unsigned int m_filterMax;

    texture();
    void generate(unsigned int width, unsigned int height, unsigned char* data);
    void bind() const;
};



#endif //TEXTURE_H
