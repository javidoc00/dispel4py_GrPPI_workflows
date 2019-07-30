
/***************************************************************************
 * samples.cpp is part of Math Graphic Library
 * Copyright (C) 2007-2016 Alexey Balakin <mathgl.abalakin@gmail.ru>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <random>
#include <experimental/optional>
#include <experimental/filesystem>
#include <complex>

//Eigen
#include <Eigen/Core>
#include <Eigen/Dense>

//mathgl
#include "mgl2/mgl.h"

// aux
#include "npy.hpp"

namespace fs = std::experimental::filesystem;

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
using TStreamR = Eigen::Array<double, Eigen::Dynamic, 1>;

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//void smgl_3wave(mglGraph &gr, TStreamR &vec)
//{
//    mglData data_y;
//    mglData data_x;
//
//    int sps=4;
//    TStreamR linspace = TStreamR::LinSpaced(vec.size(),(-vec.size()+1)/(2*sps), vec.size()/(2*sps));
//    linspace.eval();
//    data_y.Link(vec.data(),vec.size());
//    data_x.Link(linspace.data(),linspace.size());
//
//    mglGraph gr;
//    gr.SetTickTempl('x',"%.0f"); gr.SetTickTempl('y',"%.0f");
//    gr.SetTuneTicks(0);
//    gr.SetFontSizePT(6.0);
//    gr.Box("",false);
//    gr.SetRanges(-1000, 1000, (data_y.Minimal() * 1.1), (data_y.Maximal() * 1.1));
//    std::cout << "(" << data_y[0] << ", " << data_y[1] << ", " << vec.size() << ")" << std::endl;
//    gr.Axis();
//
//    gr.Plot(data_x,data_y);
//    gr.WriteEPS("prueba.eps");
//
//}

void smgl_3wave(mglGraph &gr, TStreamR &vec)
{
    mglData data_y;
    mglData data_x;

    int sps=4;
    //int size = vec.size();
    //TStreamR linspace = TStreamR::LinSpaced(size,(size+1)/(2*sps), size/(2*sps));
    TStreamR linspace = TStreamR::LinSpaced(vec.size(),(-vec.size()+1)/(2*sps), vec.size()/(2*sps));
    linspace.eval();
    data_y.Link(vec.data(),vec.size());
    data_x.Link(linspace.data(),linspace.size());

    gr.SetTickTempl('x',"%.0f"); gr.SetTickTempl('y',"%.0f");
    gr.SetTuneTicks(0);
    gr.SetFontSizePT(6.0);
    gr.Box("",false);
    gr.SetRanges(-1000, 1000, (data_y.Minimal() * 1.1), (data_y.Maximal() * 1.1));
    std::cout << "(" << data_y[0] << ", " << data_y[1] << ", " << vec.size() << ")" << std::endl;
    gr.Axis();

    gr.Plot(data_x,data_y);
    gr.WriteEPS("prueba.eps");

}



//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int main()
{
    
    std::string file("Xcorr_0_1.out.npy");
    std::vector<TStreamR> list_end(1);
    
    list_end[0]=npy::LoadArrayFromNumpy<TStreamR>(file);

    mglGraph gr;
    smgl_3wave(gr, list_end[0]);

    return 0;

    TStreamR vec;
    
    for (int i=0; i<100; i++) {
        smgl_3wave(gr, vec);
        char buf[128];
        snprintf(buf,128,"prueba%d.eps",i);
        //gr.WriteEPS(buf);

    }
    
    return 0;
}
