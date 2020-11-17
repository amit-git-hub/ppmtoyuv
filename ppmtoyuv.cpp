#include <iostream>
#include <fstream>
#include <string>
#include <memory>

class ppmtoyuv
{
public:
    ppmtoyuv():
	width(0),
	height(0)
    {}
    ppmtoyuv(const ppmtoyuv &) = delete;
    ppmtoyuv & operator=(const ppmtoyuv &) = delete;
    ppmtoyuv(ppmtoyuv &&other) noexcept
    {
        if (this != &other)
        {
            width = other.width;
            height = other.height;
            yBuf = std::move(other.yBuf);
            cbBuf = std::move(other.cbBuf);
            crBuf = std::move(other.crBuf);

            other.width = 0;
            other.height = 0;
        }
    }
    ppmtoyuv& operator=(ppmtoyuv &&other)
    {
        if (this != &other)
        {
            width = other.width;
            height = other.height;
            yBuf = std::move(other.yBuf);
            cbBuf = std::move(other.cbBuf);
            crBuf = std::move(other.crBuf);

            other.width = 0;
            other.height = 0;
        }
	    return *this;
    }

    bool writeYUV(std::string infname)
    {
        std::string outfname = infname;
        std::size_t pos = outfname.rfind("ppm");
        if (pos != std::string::npos)
            outfname.replace(pos, 3, "yuv");
        else {
            std::cout << "Input file not ppm.\n";
            return false;
        }

        std::ifstream fin(infname);
        std::ofstream fout(outfname);
        if(!fout.is_open())
        {
            std::cout << "Unable to open " << outfname << std::endl;
            return false;
        }

        if(fin.is_open())
        {
            char newline, color;
            char dummy[2];
            uint32_t depth;

            if((dummy[0]=fin.get())!='P')
            {
                fatal_error();
                return false;
            }

            if((dummy[1]=fin.get())=='6')
                color=3; // P5: grayscale binary; P6: color binary
            dummy[1] = 53;//48+5=53

            newline = fin.get();

            // get rid of those damn comments!
            if (fin.get()=='#') {
                while (fin.get()!=newline); // tolerate crap
            } else {
                fin.unget(); // wow! no crap!
            }

            fin >> width;
            fin.get(); // space after width
            fin >> height;
            fin.get(); // '\n' after height
            fin >> depth;
            fin.get(); // the damn '\n' after depth!

            std::cout << "Width: " << width << " Height: " << height << " Depth: " << depth << "\n";

            yBuf  = std::make_unique<uint8_t[]>(width*height);
            cbBuf = std::make_unique<uint8_t[]>(width*height);
            crBuf = std::make_unique<uint8_t[]>(width*height);

            for(auto i=0;i<height;i++)
            {
                for(auto j=0;j<width;j++)
                {
                    int32_t y;
                    int32_t r, g, b;
                    static const int32_t rCoeff = 306, gCoeff = 601, bCoeff = 117;
                    static const int32_t cbCoeff = 579, crCoeff = 730;
                    uint32_t index = i*width + j;
                    auto get_pixel = [](const int32_t p) {
                        return std::min<int32_t>(std::max<int32_t>(p, 0), 255);
                    };

                    r = fin.get();
                    g = fin.get();
                    b = fin.get();

                    y = get_pixel(((rCoeff*r) + (gCoeff*g) + (bCoeff*b))>>10);
                    yBuf[index] = static_cast<uint8_t>(y);
                    cbBuf[index] = static_cast<uint8_t>(get_pixel((((b - y)*cbCoeff)>>10) + 128));
                    crBuf[index] = static_cast<uint8_t>(get_pixel((((r - y)*crCoeff)>>10) + 128));
                }
            }
            fout.write(reinterpret_cast<const char *>(yBuf.get()), width*height);
            fout.write(reinterpret_cast<const char *>(cbBuf.get()), width*height);
            fout.write(reinterpret_cast<const char *>(crBuf.get()), width*height);

            fin.close();
            fout.close();

            std::cout << "PPM to YUV complete.\n";
        } else {
            std::cout << "Unable to open file " << infname << std::endl;
            return false;
        }
        return true;
    }

    void fatal_error()
    {
	    std::cout << "Fatal error: Image starts not with P\n";
	    return;
    }

    void print_usage()
    {
        std::cout << "ppmtoyuv --help\t:\tDisplay this message.\n";
        std::cout << "Usage: ppmtoyuv <inputfile>\n";
        return;
    }
private:
    uint32_t width;
    uint32_t height;

    std::unique_ptr<uint8_t[]> yBuf;
    std::unique_ptr<uint8_t[]> cbBuf;
    std::unique_ptr<uint8_t[]> crBuf;
};

int main(int argc, char *argv[])
{
    ppmtoyuv e;

    if (argc!=2)
    {
	    e.print_usage();
    	exit(-1);
    }

    if (!e.writeYUV(argv[1]))
	    std::cout << "PPM to YUV conversion failed!!!\n";

    return 0;
}

