//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_SIZE_HPP
#define FACEDEMO_SIZE_HPP
namespace face
{
    template<typename T>
    class Size
    {
    public:
        Size(T w, T h) : mWidth(w), mHeight(h) {};

        void getSize(T &w, T &h) const
        {
            w = mWidth;
            h = mHeight;
        }

        bool setSize(T w, T h)
        {
            if (equals(w, h))
            {
                return false;
            }
            mWidth  = w;
            mHeight = h;
            return true;
        }

        bool setSize(Size<T> &size)
        {
            return setSize(size.mWidth, size.mHeight);
        }

        bool equals(T w, T h) const
        {
            return mWidth == w && mHeight == h;
        }

        bool inValidSize() const
        {
            return mWidth <= 0 || mHeight <= 0;
        }

        T getWidth() const { return mWidth; };

        T getHeight() const { return mHeight; };

        bool operator>(const Size &size) const
        {
            return mWidth > size.mWidth || mHeight > size.mHeight;
        }

        bool operator==(const Size &size) const
        {
            return mWidth == size.mWidth && mHeight == size.mHeight;
        }

        bool operator!=(const Size &size) const
        {
            return mWidth != size.mWidth || mHeight != size.mHeight;
        }

        void reset(T width = 0, T height = 0)
        {
            setSize(width, height);
        }

        Size reverse()
        {
            return Size<T>(mHeight, mWidth);
        }

    private:
        T                      mWidth;
        T                      mHeight;
    };
}
#endif //FACEDEMO_SIZE_HPP
