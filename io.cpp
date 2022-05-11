#include "io.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

bool compare_postfix(string fullString, string ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

bool compare_prefix(string fullString, string begining)
{
    if (fullString.length() >= begining.length())
    {
        return (0 == fullString.compare(0, begining.length(), begining));
    }
    else
    {
        return false;
    }
}

void IoData::processDiearea(vector<pt> points)
{
    int min_x, min_y, max_x, max_y;
    Macro* m;
    //cout<<"begin points "<<points.size()<<endl;
    if(points.size()==2){//only two means that it's the two cross point, so just memorize the four number;
        this->die_x = points[0].x;
        this->die_y = points[0].y;
        max_x = points[0].x;
        max_y = points[0].y;
        if (points[0].y > points[1].y){
            this->die_y = points[1].y;
        }
        else{
            max_y = points[1].y;
        }
        if (points[0].x > points[1].x){
            this->die_x = points[1].x;
        }
        else{
            max_x = points[1].x;
        }
        this->die_height = max_y - this->die_y;
        this->die_width = max_x - this->die_x;
    }
    else{
        min_x = 2147483647;
        min_y = 2147483647;
        max_x = -2147483647;
        max_y = -2147483647;

        int j;
        for (int i = 0; i < points.size(); i++){
            if (points[i].x > max_x){
                max_x = points[i].x;
            }
            else if (points[i].x < min_x){
                min_x = points[i].x;
            }

            if (points[i].y > max_y){
                max_y = points[i].y;
            }
            else if (points[i].y < min_y){
                min_y = points[i].y;
                j = i;
            }
        }
        this->die_x = min_x;
        this->die_y = min_y;
        this->die_width = max_x - min_x;
        this->die_height = max_y - min_y;

        int clk_wise;
        if(points[(j+1)%points.size()].y==points[j%points.size()].y){
            if(points[(j+1)%points.size()].x < points[j%points.size()].x){
                clk_wise = 1;
            }else{
                clk_wise = -1;
            }
        }else{
            if(points[(j-1)%points.size()].x < points[j%points.size()].x){
                clk_wise = -1;
            }else{
                clk_wise = 1;
            }
        }
        //cout<<"clk wise "<<clk_wise<<" points "<<points.size()<<endl;
        bool left_side = (clk_wise==1) ? false : true;//
        bool down_side = true;//

        for (int i = 0; i < points.size(); i+=1, j+=clk_wise){
            //cout<<"j "<<j<<" j0 "<<points[0].x<<" "<<points[0].y<<endl;
            j = (j+points.size())%points.size();
            //cout<<"j "<<j<<endl;
            if (points[j % points.size()].x == min_x){
                left_side = true;
            }
            if (points[j % points.size()].x == max_x){
                left_side = false;
            }
            if (points[j % points.size()].y == min_y){
                down_side = true;
            }
            if (points[j % points.size()].y == max_y){
                down_side = false;
            }
            if ((points[j].y != min_y) && (points[j].y != max_y) && (points[(j+clk_wise+points.size()) % points.size()].y == points[j].y)){
                //cout<<(j-1)%points.size()<<" "<<points[j%points.size()].y<<" "<<points[(j-1)%points.size()].y<<endl;
                if((points[(j-clk_wise+points.size()) % points.size()].y==min_y && points[(j+clk_wise+points.size()) % points.size()].x==min_x) || 
                   (points[(j-clk_wise+points.size()) % points.size()].y==max_y && points[(j+clk_wise+points.size()) % points.size()].x==max_x)){
                    //cout<<"1"<<endl;
                    m = new Macro(abs(points[j].x - points[(j+clk_wise+points.size()) % points.size()].x),
                                  abs(points[(j-clk_wise+points.size()) % points.size()].y - points[j].y),
                                  min(points[j].x, points[(j+clk_wise+points.size()) % points.size()].x) - this->die_x, 
                                  min(points[(j-clk_wise+points.size()) % points.size()].y, points[j].y) - this->die_y, 
                                  true, this->macros.size() + 1);
                }
                else if(points[j].x==min_x || points[j].x==max_x){
                    //cout<<"2"<<endl;
                    m = new Macro(abs(points[j].x - points[(j+clk_wise+points.size()) % points.size()].x),
                                  abs(points[(j+2*clk_wise+points.size()) % points.size()].y - points[j].y),
                                  min(points[j].x, points[(j+clk_wise+points.size()) % points.size()].x) - this->die_x, 
                                  min(points[(j+2*clk_wise+points.size()) % points.size()].y, points[j].y) - this->die_y, 
                                  true, this->macros.size() + 1);
                    i+=2;
                    j+=2*clk_wise;
                }else{
                    //cout<<"3"<<endl;
                    m = new Macro(abs(points[j].x - points[(j+clk_wise+points.size()) % points.size()].x),
                                  abs(points[(j+2*clk_wise+points.size()) % points.size()].y - points[j].y),
                                  min(points[j].x, points[(j+clk_wise+points.size()) % points.size()].x) - this->die_x, 
                                  min(points[(j+2*clk_wise+points.size()) % points.size()].y, points[j].y) - this->die_y, 
                                  true, this->macros.size() + 1);
                }
                this->macros.push_back(m);
                    // m = new Macro(abs(points[(j - 1) % points.size()].x - points[j % points.size()].x),
                    //               abs(points[(j) % points.size()].y - max_y),
                    //               min(points[(j) % points.size()].x, points[(j - 1) % points.size()].x) - this->die_x, points[(j) % points.size()].y - this->die_y, true, this->macros.size() + 1);
                //(double w, double h, double _x, double _y, bool is_f, int i)
            }
        }
    }
}

void IoData::parseDef(ifstream &f)
{
    string linebuff, buff, buff1, buff2, buff3, buff4, buff5, buff6, buff7;
    char c;
    int intbuff;
    bool isfixed;
    Macro *m;

    stringstream linestring;

    //--parse front part
    //--version
    getline(f, linebuff, ';');
    this->version = linebuff + ";";
    //cout<<this->version;
    //--design
    f.get(c);
    getline(f, linebuff, ';');
    if(c!='\r')
        this->design = c + linebuff + ";";
    else
        this->design = linebuff + ";";
    //cout<<this->design;
    //--dbu per micron
    f.get(c);
    getline(f, linebuff, ';');
    if(c!='\r')
        this->dbu_per_micron_string = c + linebuff + ";";
    else
        this->dbu_per_micron_string = linebuff + ";";
    linestring.str(linebuff);
    linestring >> buff >> buff >> buff >> intbuff;
    this->dbu_per_micron = intbuff;
    linestring.clear();
    //cout<<this->dbu_per_micron<<endl;

    //--die area
    vector<pt> points;
    buff.clear();
    f.get(c);
    if(c!='\r')
        buff+=c;
    f.get(c);
    if(c!='\r')
        buff+=c;
    f.get(c);
    if(c!='\r')
        buff+=c;
    getline(f, linebuff, ';');
    this->die_area_string = buff + linebuff + ";" + "\n";
    //cout<<this->die_area_string;
    linestring.str(linebuff);
    linestring >> buff >> buff;
    while (!linestring.eof())
    {
        while (buff != "(")
        {
            linestring >> buff;
        }
        linestring >> buff1 >> buff2 >> buff3;
        buff = "";
        linestring >> buff;
        //cout<<buff1<<" "<<buff2<<endl;
        pt point;
        point.x = stoi(buff1);
        point.y = stoi(buff2);
        points.push_back(point);
    }
    processDiearea(points);
    linestring.clear();

    //--parse macros
    getline(f, linebuff, ';');
    linestring.str(linebuff);
    linestring >> buff >> intbuff;
    this->num_macro = intbuff;
    linestring.clear();
    //cout<<this->num_macro<<endl;
    for (int i = 0; i < this->num_macro; i++){
        getline(f, linebuff, ';');
        linestring.str(linebuff);
        linestring >> buff >> buff1 >> buff2;                  //                 - A1/m1 c5
        linestring >> buff >> buff3 >> buff >> buff4 >> buff5; //     + FIXED ( 0 20000 ) N ;
        //cout<<buff1<<" "<<buff2<<" "<<buff3<<" "<<buff4<<" "<<buff5<<" "<<endl;
        if (buff3 == "FIXED"){
            intbuff = _fixed;
            isfixed = true;
        }
        else if (buff3 == "PLACED"){
            intbuff = placed;
            isfixed = false;
        }
        else{
            cerr << "Undefined macro movable type: " << buff3;
        }

        //(string name, string shape, int type, double _x, double _y, bool is_f, int i)
        m = new Macro(buff1, buff2, intbuff, stod(buff4.c_str()) - this->die_x, stod(buff5.c_str()) - this->die_y, isfixed, this->macros.size() + 1);
        this->macros.push_back(m);

        linestring.clear();
    }
}

void IoData::parseLef(ifstream &f){
    string linebuff, buff, buff1, buff2, buff3, buff4, buff5, buff6, buff7;
    int intbuff;
    bool isfixed;
    Macro *m;
    //cout<<"lef fuck" ;

    stringstream linestring;

    while (!f.eof()){
        getline(f, linebuff);
        if (linebuff == "END LIBRARY"){
            break;
        }
        linestring.str(linebuff);
        linestring >> buff >> buff1;
        linestring.clear();

        getline(f, linebuff);
        linestring.str(linebuff);
        linestring >> buff >> buff2 >> buff >> buff3;
        linestring.clear();
        //cout<<buff1<<" "<<buff2<<" "<<buff3<<endl;

        m = new Macro(buff1, stod(buff2), stod(buff3));
        this->macro_shapes.push_back(m);

        getline(f, linebuff);
        getline(f, linebuff);
    }

}

//--------------
//five argument in txt
void IoData::parseTxt(ifstream &f){
    string linebuff, buff, buff1, buff2, buff3, buff4, buff5, buff6, buff7;
    int intbuff;

    stringstream linestring;

    getline(f, linebuff);
    linestring.str(linebuff);
    linestring >> buff >> intbuff;
    this->powerplan_width_constraint = intbuff;
    linestring.clear();

    getline(f, linebuff);
    linestring.str(linebuff);
    linestring >> buff >> intbuff;
    this->minimum_spacing = intbuff;
    linestring.clear();

    getline(f, linebuff);
    linestring.str(linebuff);
    linestring >> buff >> intbuff;
    this->buffer_constraint = intbuff;
    linestring.clear();

    getline(f, linebuff);
    linestring.str(linebuff);
    linestring >> buff >> intbuff;
    this->weight_alpha = intbuff;
    linestring.clear();

    getline(f, linebuff);
    linestring.str(linebuff);
    linestring >> buff >> intbuff;
    this->weight_beta = intbuff;
    linestring.clear();
}

void IoData::output()
{
    ofstream f(this->output_filename.c_str());
    if (!f.good())
    {
        cerr << "Unable to output file";
    }

    f << this->version;
    f << this->design;
    f << this->dbu_per_micron_string;
    f << this->die_area_string;

    f << "\nCOMPONENTS " << this->num_macro << " ;\n";

    for (int i = 0; i < this->macros.size(); i++)
    {
        if (this->macros[i]->type() == border)
        {
            continue;
        }
        f << "   - " << this->macros[i]->name() << " " << this->macros[i]->shape() << " \n"
          << "      + ";
        if (this->macros[i]->is_fixed())
        {
            f << "FIXED ( ";
        }
        else
        {
            f << "PLACED ( ";
        }
        f.precision(dbl::max_digits10);
        f << this->macros[i]->x1() << " " << this->macros[i]->y1() << " ) N ;\n";
    }

    f << "END COMPONENTS\n\n\nEND DESIGN\n\n\n";
    f.close();
}

IoData::IoData(int argc, char *argv[]){
    ifstream fs;
    string arg[argc];

    //get argv and call parse
    for (int i = 1; i < argc; i++)
    {
        arg[i] = argv[i];
        if (compare_prefix(arg[i], "out"))
        {
            this->output_filename = arg[i];
            continue;
        }

        fs.open(argv[i]);
        if (!fs.good())
        {
            cerr << "Unable to open " << argv[i] << endl;
        }

        if (compare_postfix(arg[i], "lef"))
        {
            this->parseLef(fs);
        }
        else if (compare_postfix(arg[i], "def"))
        {
            this->parseDef(fs);
        }
        else if (compare_postfix(arg[i], "txt"))
        {
            this->parseTxt(fs);
        }
        else
        {
            cerr << "File type unavailable: " << argv[i] << endl;
        }

        fs.close();
    }

    for (int i = 0; i < this->macros.size(); i++)
    {
        if (this->macros[i]->type() == border)
        {
            continue;
        }

        for (int j = 0; j < this->macro_shapes.size(); j++)
        {
            if (this->macros[i]->shape() == this->macro_shapes[j]->name())
            {
                this->macros[i]->setWidthHeight(*(this->macro_shapes[j]), this->dbu_per_micron);
                break;
            }
        }
    }

    this->powerplan_width_constraint *= this->dbu_per_micron;
    this->minimum_spacing *= this->dbu_per_micron;
    this->buffer_constraint *= this->dbu_per_micron;
}