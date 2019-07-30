#include <iostream>
#include <string>
#include <vector>
#include <experimental/optional>
#include <experimental/filesystem>
#include <complex>
#include <iomanip>
//Eigen
#include <Eigen/Core>
#include <Eigen/Dense>

//mathgl
#include "mgl2/mgl.h"

#include "npy.hpp"
#include "fftw_itf.hpp"
#include "eigen_shift.hpp"

namespace ex = std::experimental;
namespace fs = std::experimental::filesystem;

using TStreamC = Eigen::Array<std::complex<double>, Eigen::Dynamic, 1>;
using TStreamR = Eigen::Array<double, Eigen::Dynamic, 1>;


int main()
{

    fs::path file_temp = fs::path("temp.npy");

    fs::path file1 = fs::path("prueba1.npy");
    fs::path file2 = fs::path("prueba2.npy");
    fs::path file_fig = fs::path("figure.eps");

    TStreamC data1 = npy::LoadArrayFromNumpy<TStreamC>(file1);
    TStreamC data2 = npy::LoadArrayFromNumpy<TStreamC>(file2);

    int shift = 5000;

    auto size1 = data1.size();
    auto size2 = data2.size();
    auto size = (size1 < size2) ? size1 : size2;

    std::cout << std::setprecision(9) <<  "data1: " << data1(1234) << std::endl;
    std::cout << std::setprecision(9) <<  "data2: " << data2.conjugate()(1234) << std::endl;

    TStreamC temp1 = data1.head(size) * data2.head(size).conjugate();
    
    //npy::SaveArrayAsNumpy(file_temp, temp1);
    //TStreamC temp1 = npy::LoadArrayFromNumpy<TStreamC>(file_temp);

    std::cout << std::setprecision(9) <<  "temp1: " << temp1(1234) << std::endl;

    TStreamC temp2 = fftw_itf::ifft(temp1) / size;

    std::cout << std::setprecision(9) <<  "ifft: " << temp2(1234) << std::endl;

    TStreamR temp3 = temp2.real();
    std::cout << std::setprecision(9) <<  "real: " << temp3(1234) << std::endl;

    TStreamR temp4 = eigen_shift::vShift(temp3,shift);
    TStreamR figure = temp4.head(2 * shift + 1);
    std::cout << std::setprecision(9) <<  "roll: " << figure(1234) << std::endl;
    std::cout << std::setprecision(9) <<  "roll: " << figure(10) << std::endl;
    std::cout << std::setprecision(9) <<  "roll: " << figure(2000) << std::endl;

    size = figure.size();
    
    // set data for graph
    mglData data_y;
    mglData data_x;

    // calculate linspace
    int sps=4;
    TStreamR linspace = TStreamR::LinSpaced(size,-(size+1)/(2*sps), size/(2*sps));
    linspace.eval();
 
    data_y.Link(figure.data(),figure.size());
    data_x.Link(linspace.data(),linspace.size());

    // set grph attibutes
    mglGraph gr;
    gr.SetTickTempl('x',"%.0f"); gr.SetTickTempl('y',"%.0f");
    gr.SetTuneTicks(0);
    gr.SetFontSizePT(6.0);
    gr.Box("",false);
    gr.SetRanges(-1000, 1000, (data_y.Minimal() * 1.1), (data_y.Maximal() * 1.1));

    // draw and write graph
    gr.Axis();
    gr.Plot(data_x,data_y);
    gr.WriteEPS(file_fig.c_str());


    return 0;
}
