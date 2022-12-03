#include<cstdio>
#include<algorithm>
#include<iostream>
#include<cmath>
#include<string.h>
#include<fstream>
#include<vector>
#include<queue>
#include<memory.h>
#include <map>                                              //store memory data

using namespace std;




struct Inst{                                                 // Instrcution struction
    string Itpye = "";                                      // instruction type R、I、J
    string op = "";                                         // instruction operation
    string instr_b = "";
    string assmble = "";
    int address = 0;                                        // memory address
    int rs = 0;                                             //
    int rt = 0;
    int rd = 0;
    int immd = 0;
    int shamt = 0;
    int funct = 0;

    int des = 0;                            //目标寄存器
};

struct Function_uint{
    string name = "";
    string Busy = "";
    string Op = "";
    string Fi = "";
    string Fj = "";
    string Fk = "";
    string Qj = "";
    string Qk = "";
    string Rj = "";
    string Rk = "";
    
};

Function_uint F_U_S[4];
string Register_status[33] = {""};                ///rigister status


string Function_uints[3] = {"MEM", "ALU","ALUB"};
//Inst* inst = nullptr;

int read_file(char* file);                          //read binary code

Inst* Decode_inst(string cur_inst);               //instruction decode
int string_32b_To_int(string input);             //32bit string change to unsigned int
void Decode();                                  // binary code  decode
int Excute(Inst * cur, int flag);                                  //instruction execute
void Init();                                    // register Init
//void Inst_print(Inst* inst);
void Inst_print(Inst* inst, ofstream &outFile);
void Register_print();
void Data_print();
void pipeline_excute();
void pipeline_print();
void register_status_print();

void statue_generate();
void instruction_fetch();
void Issue();
void MEM();
void ALU();

void ALUB();
void WB();
int count = 0;                                      ///code line 
int PC = 0;                                         //program count
int cycle = 0;                                      //programe cycle

int is_fetch_stop = 0;                          //fetch stop 
int ALUB_count = 0;
int program_stop = 0;                         //program stop

string waiting_instruction[2];                 //0 stand for new, 1 stand for last.
string Execute_instruction[2];

string line;  
string first_bit;          
string op, rs, rt, rd, shamt, funct, imm;
vector<string> bin_set;                              // binart set
//Inst  instruction ;
//inst = &instruction;
vector<Inst*> Inst_set;                             // instrction set

vector<Inst*> Pre_Issue_vector,Pre_Issue_vector_last;
vector<Inst*> Pre_MEM_vector,Pre_MEM_vector_last;
vector<Inst*> Pre_ALU_vector,Pre_ALU_vector_last;
vector<Inst*> Pre_ALUB_vector,Pre_ALUB_vector_last;
vector<Inst*> Post_MEM_vector,Post_MEM_vector_last;
vector<Inst*> Post_ALU_vector,Post_ALU_vector_last;
vector<Inst*> Post_ALUB_vector,Post_ALUB_vector_last;

//vector<Inst*> instruction_status;



ofstream outFile;	                                //define ofstream object outFile
ofstream outFile_simu;
int Register[32];
map<int,int> mem_map;
int mem_start,mem_end;                              //record the start and end of the memory data


int main(int argc, char* argv[])
{
	//instruction  = new Inst();
    outFile.open("disassmbly.txt");

    //outFile<<argc<<endl;
    if(argc == 3 )
        outFile_simu.open(argv[2]);
    else
        outFile_simu.open("simulation.txt");
    //outFile_simu.open("simulation.txt");
    int return_code = 1;

    Init();
    return_code = read_file(argv[1]);
    
    if(return_code == 0)
        return 0;
    Decode();

    PC = 0;
    pipeline_excute();

    outFile.close();
    outFile_simu.close();
	return 0;
}


void pipeline_excute()
{
    
    while(!program_stop)
    {

        instruction_fetch();
        Issue();
        MEM();
        ALU();
        ALUB();
        WB();
        statue_generate();
        pipeline_print();


      //  register_status_print();
        cycle++;
    }

    //cycle++;
}

int pre_issue_buffer_check(int res_num)
{
    int res = 0;
    for(int i = 0; i< Pre_Issue_vector_last.size(); i++)
    {
        if(Pre_Issue_vector_last[i]->des == res_num)
        {   
            res = 1;
            break;
        }
    }
    return res;
}

void statue_generate()
{
    Pre_Issue_vector_last = Pre_Issue_vector;
    Pre_MEM_vector_last = Pre_MEM_vector;
    Pre_ALU_vector_last = Pre_ALU_vector;
    Pre_ALUB_vector_last = Pre_ALUB_vector;
    Post_MEM_vector_last = Post_MEM_vector;
    Post_ALU_vector_last = Post_ALU_vector;
    Post_ALUB_vector_last = Post_ALUB_vector;
}
void instruction_fetch()
{
    int i;
    Execute_instruction[0] = "";
    waiting_instruction[0] = "";

  for(i = 0; i < 2; i++)
  {
    if(Pre_Issue_vector.size() == 4)
            break;
    if(Inst_set[PC]->op == "BREAK")                                //取到BREAK指令 程序停止
    {   
        Execute_instruction[0] = Inst_set[PC]->op;
        program_stop = 1;
        break;
    }
    else if(Inst_set[PC]->op == "J"||Inst_set[PC]->op == "JR" ||Inst_set[PC]->op == "BEQ"||Inst_set[PC]->op == "BLTZ" ||Inst_set[PC]->op == "BGTZ"  )
    {
        if(Inst_set[PC]->op == "JR" ||Inst_set[PC]->op == "BLTZ"||Inst_set[PC]->op == "BGTZ")
        {
            if(Register_status[Inst_set[PC]->rs] == "" && !pre_issue_buffer_check(Inst_set[PC]->rs))
            {
                Execute_instruction[0] = Inst_set[PC]->assmble;

                Excute(Inst_set[PC], 0);
            }
            else
            {
                waiting_instruction[0] = Inst_set[PC]->assmble;
            }
        }
        else if(Inst_set[PC]->op == "J")
        {
            Execute_instruction[0] = Inst_set[PC]->assmble;

            Excute(Inst_set[PC],0);
           // break;
        }
        else
        {
            if(Register_status[Inst_set[PC]->rs] == "" && Register_status[Inst_set[PC]->rt] == ""&&!pre_issue_buffer_check(Inst_set[PC]->rs) && !pre_issue_buffer_check(Inst_set[PC]->rt))
            {
                Execute_instruction[0] = Inst_set[PC]->assmble;

                Excute(Inst_set[PC], 0);
                //Execute_instruction[0] = Inst_set[PC]->assmble;
             //   PC++;
            }
             else
            {
                waiting_instruction[0] = Inst_set[PC]->assmble;
            }
        }
           break;

    }
    else if(Pre_Issue_vector.size() < 4)
    {
        Pre_Issue_vector.push_back(Inst_set[PC]);
        PC++;
        //if(Inst_set[PC]->op != "")
     //   Register_status[Inst_set[PC]->des] = Inst_set[PC]->op;
        if(Pre_Issue_vector.size() == 4)
            break;
    }
    
  }
  
}


void Issue()
{   int i;
    int is_issue[4] = {0, 0, 0, 0};
    string register_change_content[4] = {"", "", "", ""};
    int register_change_index[4] = {0, 0, 0, 0};
    ///Pre_Issue_vector = Pre_Issue_vector_last;
    for(i = 0; i < Pre_Issue_vector_last.size(); i++)
    {

        Inst * temp = Pre_Issue_vector_last[i];
        

        //outFile_simu<<"temp->op"<< temp->op <<" des"<<temp->des<<endl;

            if(temp->op == "SLL"||temp->op == "SRL"||temp->op == "SRA"||temp->op == "MUL")
            {
                
                if(Excute(temp,1))
                {
                    if(Pre_ALUB_vector_last.size() < 2)
                    {
                        Pre_ALUB_vector.push_back(temp);
                        is_issue[i] = 1;
                    
                    }
                }
                register_change_index[i] = temp->des;
                register_change_content[i] = Register_status[temp->des];
                Register_status[temp->des] = temp->op;
            }
            else if(temp->op == "LW"||temp->op == "SW")
            {
                
                if(Excute(temp,1))
                {
                    if(Pre_MEM_vector.size() < 2)
                    {
                        Pre_MEM_vector.push_back(temp);
                        is_issue[i] = 1;
                      //  if(temp->op == "LW")
                          //  Register_status[temp->rs] = "LW";
                    }
                }

                if(temp->op == "LW")
                {
                    register_change_index[i] = temp->rt;
                    register_change_content[i] = Register_status[temp->rt];
                    Register_status[temp->rt] = temp->op;
                }
                else
                {
                    
                }
            }
            else
            {
                
                if(Excute(temp,1))
                {
                    if(Pre_ALU_vector_last.size() < 2)
                    {
                        is_issue[i] = 1;
                        Pre_ALU_vector.push_back(temp);
                       //<<"temp->op"<< temp->op <<" des"<<temp->des<<endl;
                       // Register_status[temp->des] = temp->op;
                    }
                }

                register_change_index[i] = temp->des;
                register_change_content[i] = Register_status[temp->des];
                Register_status[temp->des] = temp->op;
            }
        
    }
    //发射出去的  进行清除。
    for(i = Pre_Issue_vector_last.size() - 1; i >= 0; i--)
    {
        if(is_issue[i])
            Pre_Issue_vector.erase(Pre_Issue_vector.begin() + i);
        else
        {
            Register_status[register_change_index[i]] = register_change_content[i];
        }

    }


    Pre_Issue_vector_last = Pre_Issue_vector;
}
void MEM()
{
    if(Post_MEM_vector.size() > 0)
        Post_MEM_vector.erase( Post_MEM_vector.begin() + 0);

    if(Pre_MEM_vector_last.size() != 0)
    {
        if(Pre_MEM_vector_last[0]->op == "SW")
        {
            Excute(Pre_MEM_vector_last[0], 0);
        }
        else
        {   
            Post_MEM_vector.push_back(Pre_MEM_vector_last[0]);
            //Pre_MEM_vector.erase(Pre_MEM_vector.begin()+ 0);
        }
        Pre_MEM_vector.erase(Pre_MEM_vector.begin()+ 0);

    }
}
void ALU()
{
   // outFile_simu<<Pre_ALU_vector_last.size()<<endl;
      //  outFile_simu<<Pre_ALU_vector.size()<<endl;

    // if(Pre_ALU_vector_last.size() > 0)
    // {
    //     Post_ALU_vector[0] = Pre_ALU_vector_last[0];
    //             outFile_simu<<"PostALu"<<Pre_ALU_vector.size()<<endl;

    //     Pre_ALU_vector.erase(Pre_ALU_vector.begin()+ 0);
    //             outFile_simu<<"Pre_alu"<<Pre_ALU_vector.size()<<endl;


    // }
    if(Post_ALU_vector.size() > 0)
        Post_ALU_vector.erase( Post_ALU_vector.begin() + 0);

    if(Pre_ALU_vector_last.size() != 0)
    {
        //Inst * temp = Pre_ALU_vector_last[0];
        Post_ALU_vector.push_back(Pre_ALU_vector_last[0]);
        Pre_ALU_vector.erase(Pre_ALU_vector.begin()+ 0);
    }
//    outFile_simu<<"post_alu"<<Post_ALU_vector.size()<<endl;


}
void ALUB()
{
    if(Post_ALUB_vector.size() > 0)
        Post_ALUB_vector.erase( Post_ALUB_vector.begin() + 0);

    if(Pre_ALUB_vector_last.size() != 0)
    {
        ALUB_count++;
        if(ALUB_count == 2)
        {
            Post_ALUB_vector.push_back(Pre_ALUB_vector_last[0]);
            Pre_ALUB_vector.erase(Pre_ALUB_vector.begin()+ 0);
            ALUB_count = 0;
        }
        else
        {

        }
    }
}
void WB()
{
    if(Post_MEM_vector_last.size()>=1)
    {
        Excute(Post_MEM_vector_last[0], 0);
        Register_status[Post_MEM_vector_last[0]->rs] = "";
       
        if(Post_MEM_vector_last[0]->op == "LW")
        {
            Register_status[Post_MEM_vector_last[0]->rt] = "";
        }
        else
        {

        }
       //  Post_MEM_vector.erase(Post_MEM_vector.begin()+ 0);
    }
    if(Post_ALU_vector_last.size()>=1)
    {
        Excute(Post_ALU_vector_last[0], 0);
        Register_status[Post_ALU_vector_last[0]->des] = "";
       // Post_ALU_vector.erase(Post_ALU_vector.begin()+ 0);
    }
    if(Post_ALUB_vector_last.size()>=1)
    {
        Excute(Post_ALUB_vector_last[0], 0);
        Register_status[Post_ALUB_vector_last[0]->des] = "";
     //   Post_ALUB_vector.erase(Post_ALUB_vector.begin()+ 0);
    }
}
void register_status_print()
{
    outFile_simu<<"register status"<<" ";
    for(int i = 0; i < 33; i++)
    {
        outFile_simu<<i<<" "<<Register_status[i]<<" ";
    }
}

void pipeline_print()
{
    outFile_simu<<"--------------------"<<endl;

    outFile_simu<<"Cycle:"<<cycle + 1<<endl;
    outFile_simu<<endl;
    int i = 0;
    outFile_simu<<"IF Unit:"<<endl;
    outFile_simu<<"\tWaiting Instruction: ";
   
    outFile_simu<<waiting_instruction[0]<<endl;
    outFile_simu<<"\tExecuted Instruction: ";
    outFile_simu<<Execute_instruction[0]<<endl;
    outFile_simu<<"Pre-Issue Buffer:"<<endl;
    for(i = 0; i < 4; i++)
    {
        outFile_simu<<"\tEntry "<<i<<":";
        if(Pre_Issue_vector.size() >= i + 1)
            outFile_simu<<"["<<Pre_Issue_vector[i]->assmble<<"]"<<endl;
        else
            outFile_simu<<endl;
    }
    outFile_simu<<"Pre-ALU Queue:"<<endl;
    for(i = 0; i < 2; i++)
    {
        outFile_simu<<"\tEntry "<<i<<":";
        if(Pre_ALU_vector.size() >= i + 1)
            outFile_simu<<"["<<Pre_ALU_vector[i]->assmble<<"]"<<endl;
        else
            outFile_simu<<endl;
        
    }

    outFile_simu<<"Post-ALU Buffer:";
    //if(Post_ALU_vector)
    if(Post_ALU_vector.size() >= 1 )
        outFile_simu<<"["<<Post_ALU_vector[0]->assmble<<"]"<<endl;
    else   
        outFile_simu<<endl;

    outFile_simu<<"Pre-ALUB Queue:"<<endl;
    for(i = 0; i < 2; i++)
    {
        outFile_simu<<"\tEntry "<<i<<":";
        
        if(Pre_ALUB_vector.size() >= i + 1)
            outFile_simu<<"["<<Pre_ALUB_vector[i]->assmble<<"]"<<endl;
        else
            outFile_simu<<endl;
    }
    outFile_simu<<"Post-ALUB Buffer:";
    
    if(Post_ALUB_vector.size() >= 1 )
        outFile_simu<<"["<<Post_ALUB_vector[0]->assmble<<"]"<<endl;
    else
        outFile_simu<<endl;
    outFile_simu<<"Pre-MEM Queue:"<<endl;
    for(i = 0; i < 2; i++)
    {
        outFile_simu<<"\tEntry "<<i<<":";
        if(Pre_MEM_vector.size() >= i + 1)
            outFile_simu<<"["<<Pre_MEM_vector[i]->assmble<<"]"<<endl;
        else
            outFile_simu<<endl;
    }
    outFile_simu<<"Post-MEM Buffer:";
    if(Post_MEM_vector.size() >= 1)
        outFile_simu<<"["<<Post_MEM_vector[0]->assmble<<"]"<<endl;
    else
        outFile_simu<<endl;
    outFile_simu<<endl;
    Register_print();
    Data_print();
}
void Init()
{
    int i;
    memset(Register, 0, sizeof(int)*32);  // init the 32 Register

    for(i = 0; i < 3; i++)
    {
        F_U_S[i].name = Function_uints[i];
        F_U_S[i].Busy = "No";
    }
    for(i = 0; i < 33; i++)
    {
        Register_status[i] = "";
    }
}


int read_file(char* file)
{
    ifstream in(file);   //读取文件
    string filename;        
    int state;              
    int count = 0;
    if(in) // 有该文件
    {
        while (getline (in, line)) // line中不包括每行的换行符
        { 
            bin_set.push_back(line);
            count++;
        }
        return 1;
    }
    else // 没有该文件
    {   
      //  cout <<"no such file" << endl;
        return 0;
    }
}


void Decode()
{

    int state = 0;
    for(PC = 0; PC < bin_set.size(); PC++)
    {
        line = bin_set[PC];
        if(state != 1)
        {
            Inst* inst1 = Decode_inst(line);
            Inst_set.push_back(inst1);
            state = inst1->op == "BREAK" ? 1 : 0;
            if(state == 1)
            {
                mem_start = 64+PC * 4 + 4;
            }
        }
        else
        {   
            line = line.substr(0, 32);                              //attention！！
            outFile<<line<<"\t";
            mem_map[64+PC*4] = string_32b_To_int(line);
            if(PC != bin_set.size() - 1)
                outFile<<64+PC*4<<"\t"<<string_32b_To_int(line)<<endl;
            else
                outFile<<64+PC*4<<"\t"<<string_32b_To_int(line);

        }
        
    }

    mem_end = 64 + (bin_set.size() - 1) * 4;
    // for(int ii = mem_start ; ii<=mem_end;ii = ii + 4)
    // {
    //     cout<<mem_map[ii]<<" ";
    // }
}



Inst* Decode_inst(string cur_inst)
{


    Inst*  inst = new Inst();
    // get the components of the code
    first_bit = cur_inst.substr(0, 1);
    op = cur_inst.substr(1, 5);
    int operation = stoi(op, 0, 2);


    rs = cur_inst.substr(6,5);
    rt = cur_inst.substr(11,5);
    rd = cur_inst.substr(16,5);
    shamt = cur_inst.substr(21,5);
    funct = cur_inst.substr(26, 6);
    int rs_int = stoi(rs, 0, 2);
    int rt_int = stoi(rt, 0, 2);
    int rd_int = stoi(rd, 0, 2);
    int shamt_int = stoi(shamt, 0, 2);
    int function = stoi(funct, 0, 2);

    inst->rs = rs_int;
    inst->rt = rt_int;
    inst->rd = rd_int;
    inst->shamt = shamt_int;
    inst->funct = function;
    inst->address = 64 + PC * 4;
    outFile<<first_bit<<" "<<op<<" "<<rs<<" "<<rt<<" "<<rd<<" "<<shamt<<" "<<funct<<"\t";
    outFile<<inst->address<<"\t";

    //cout<<first_bit<<op<<" "<<rs<<" "<<rt<<" "<<rd<<" "<<shamt<<" "<<funct<<"\t";
    //cout<<64+PC*4<<"\t";

   //         -----------R type------------------
    if(operation == 0 || operation == 0b11100)
    {

        inst->Itpye = "R";
   
        
        inst->des = inst->rd;

        if(operation == 0)
        {
            //inst->des = inst->rd;
            switch (function)
            {
            case  0b001000:
                inst->op = "JR";

                /* code */
                break;
            case  0b001101:
                inst->op = "BREAK";
                /* code */
                break;
            case  0b000000:
                if(rs_int == 0 && rt_int && 0 && rd_int == 0 && shamt_int == 0)
                {
                    inst->op = "NOP";
                }
                else
                {
                    inst->op = "SLL";
                   
                }
                /* code */
                break;

            case  0b000010:
                
                inst->op = "SRL";
                /* code */
                break;
            case  0b000011:
                
                inst->op = "SRA";
                /* code */
                break;
            case  0b100000:
                inst->op = "ADD";
                /* code */
                break;
            case  0b0100010:
                
                inst->op = "SUB";
                /* code */
                break;
            case  0b100100:
                
                inst->op = "AND";
                /* code */
                break;

            case  0b100111:
                
                inst->op = "NOR";
                /* code */
                break;

            case  0b101010:
                
                inst->op = "SLT";
                /* code */
                break;
            default:
                break;
            }
        }
        else if (operation == 0b11100)
        {
             inst->op = "MUL";
        }
    }

    // //         -------------------J type-------------------------
    else if(operation == 0b00010)
    {
       inst->Itpye = "J";
       imm = cur_inst.substr(16,16);

        int imm_int = stoi(imm, 0, 2);

        inst->immd = imm_int;

        switch (operation)
        {

            //J型
            case 0b00010:
                /* code */
                inst->op = "J";
                break;
            

            default:
                break;
        }
    }
     //         -----------I type------------------
    else 
    {   
        inst->Itpye = "I";
        rs = cur_inst.substr(6,5);
        rt = cur_inst.substr(11,5);
        imm = cur_inst.substr(16, 16);

        int rs_int = stoi(rs, 0, 2);
        int rt_int = stoi(rt, 0, 2);
        int imme_int = stoi(imm, 0, 2);
            
        inst->rs = rs_int;
        inst->rt = rt_int;
        inst->immd = imme_int;

        if(stoi(first_bit) == 1)                    //the special imme instruction ：first bit == 1
        {
            inst->des = inst->rt;
            switch (operation)
            {
               //inst->des = inst->rt;

                //MUL 
                case 0b00001:
                    /* code */
                    inst->op = "MUL";
                    break;   

                //SW 
                case 0b01011:
                    /* code */
                    inst->op = "SW";
                    
                    break; 
                //LW 
                case 0b00011:
                    /* code */
                    inst->op = "LW";
                    
                    break; 

                //ADD 
                case 0b10000:
                    /* code */
                    inst->op = "ADD";
                    break;
                //SUB
                case 0b10001:
                    /* code */
                    inst->op = "SUB";
                    break;
                //AND
                case 0b10010:
                    /* code */
                    inst->op = "AND";
                    break;
                //NOR
                case 0b10011:
                    /* code */
                    inst->op = "NOR";
                    break;
                //SLT
                case 0b10101:
                    /* code */
                    inst->op = "SLT";
                    break;
                default:
                    break;
            }

        }
        else                                            //the normal imme instruction 
        {
            switch (operation)
            {
                //BEQ型   I
                case 0b00100:
                    inst->op = "BEQ";

                    /* code */
                    break;

                //BLTZ型  I
                case 0b00001:
                    /* code */
                    inst->op = "BLTZ";
                    break;

                //BGTZ型  I
                case 0b00111:
                inst->op = "BGTZ";
                    /* code */
                    break;
            }
        }
    }
    Inst_print(inst, outFile);
    return inst;
}

int Excute(Inst * cur, int flag)
{
    int branch = 0;
     Inst* inst = cur;
    if(flag == 0)
    {//PC = 0;
   // int branch = 0;                     //is program  jump 
    
    
    branch = 0;

    //Inst_print(Inst_set[PC], outFile_simu); 
    //outFile_simu <<endl;
    
    }
    if(inst->Itpye == "R")
    {
        if(inst->op == "BREAK")
        {

        }

        else if(inst->op == "SLL"||inst->op == "SRL")
        {
            if(!flag)
            {
                if(inst->op == "SLL")
                {
                    //if(!flag)
                        Register[inst->rd] = Register[inst->rt] << inst->shamt;
                }
                else if(inst->op == "SRL")
                {
                    Register[inst->rd] = Register[inst->rt] >> inst->shamt;
                }
            }
            else
            {
                if(Register_status[inst->rt] != "")
                        {
                            return 0;
                        }
                        else
                            return 1;
            }

        }
        else
        {

            if(!flag)
            {
                if(inst->op == "ADD")
                {
                    Register[inst->rd] = Register[inst->rs] + Register[inst->rt];
                }
                else if(inst->op == "SUB")
                {
                    Register[inst->rd] = Register[inst->rs] - Register[inst->rt];
                }
                else if(inst->op == "MUL")
                {
                    Register[inst->rd] = Register[inst->rs] * Register[inst->rt];
                }
                else if(inst->op == "AND")
                {
                    Register[inst->rd] = Register[inst->rs] && Register[inst->rt];
                }
                else if(inst->op == "NOR")
                {
                    Register[inst->rd] = !(Register[inst->rs] || Register[inst->rt]);
                }
                else if(inst->op == "SLT")
                {
                    Register[inst->rd] = (Register[inst->rs] < Register[inst->rt]) ? 1 : 0;
                }
                else if(inst->op == "JR")           //JR rs
                {
                    PC = Register[inst->rs];
                    branch = 1;
                }
                else if(inst->op =="SRA")     //signed shift
                {
                    Register[inst->rd] = (unsigned int)Register[inst->rt] >> inst->shamt;
                }
                else if(inst->op == "NOP")
                {

                }
            }
            else
            {
                if(inst->op == "SRA")
                {
                    if(Register_status[inst->rt] != "")
                        return 0;
                    else
                        return 1;
                }
                else
                {
                    if(Register_status[inst->rs] == ""&&Register_status[inst->rt] == "")
                        return 1;
                    else 
                        return 0;
                }
            }
            return 0;
        }
    }
    else if(inst->Itpye == "J")
    {
        if(inst->op == "J")
        {
            PC = (inst->immd *4 - 64) / 4;
            branch = 1;
        }
    }
    else
    {
       // if(!flag)
       // {
            if(inst->op == "ADD" ||inst->op == "SUB" || inst->op == "MUL"||inst->op == "AND"||inst->op == "NOR"||inst->op == "SLT"||inst->op == "BEQ")
            {
                if(!flag)
                {

                    if(inst->op == "ADD")     // rt = rs +  immd
                    {    
                        Register[inst->rt] = Register[inst->rs] + inst->immd;
                     //   inst->des = inst->rt;
                        //outFile<<inst->op<<" "<<"R"<<inst->rs<<", R"<<inst->rt<<", #"<<inst->immd*4<<endl;
                    }
                    else if(inst->op == "SUB")
                    {                   
                        Register[inst->rt] = Register[inst->rs] - inst->immd;

                    }
                    else if(inst->op == "MUL")
                    {
                        Register[inst->rt] = Register[inst->rs] * inst->immd;
                    }
                    else if(inst->op == "AND")
                    {
                        Register[inst->rt] = Register[inst->rs] && inst->immd;
                    }
                    else if(inst->op == "NOR")
                    {
                        Register[inst->rt] = !(Register[inst->rs] || inst->immd);
                    }
                    else if(inst->op == "SLT")
                    {
                        Register[inst->rt] = (Register[inst->rs] < inst->immd)? 1: 0;
                    }
                    
                    else if(inst->op == "BEQ")
                    {
                        if(Register[inst->rs] == Register[inst->rt])
                        {
                            PC += inst->immd;
                            PC++;
                            branch = 1;
                        }
                        else
                            PC++;
                    }
                    

                    else
                    {

                    } 
                
                }   
                else 
                {
                    if(Register_status[inst->rs] != "")
                        return 0;
                    else
                        return 1;
                }

            }    
        else if(inst->op == "BGTZ"||inst->op =="BLTZ" )
        {
            if(!flag)
            {
                if(inst->op == "BLTZ")
                {
                    if(Register[inst->rs] < 0)
                    {
                        PC += inst->immd;
                        PC++;
                        branch = 1;
                    }
                    else
                        PC++;
                }
                else if(inst->op == "BGTZ")
                {
                    if(Register[inst->rs] > 0)
                    {
                        PC += inst->immd;
                        PC++;
                        branch = 1;
                    }
                    else
                        PC++;
                }
            }

        }
        else if(inst->op == "LW" || inst->op == "SW")
        {
            if(!flag)
            {
                if(inst->op == "SW")
                {
                    mem_map[Register[inst->rs] + inst->immd] = Register[inst->rt];
                }
                else if(inst->op == "LW")
                {
                    Register[inst->rt] = mem_map[Register[inst->rs] + inst->immd];
                }
            }
            else
            {
                if(inst->op == "LW")
                {
                    if(Register_status[inst->rs] =="" && Register_status[inst->rt] =="" )
                        return 1;
                    else
                    return 0;
                }
                else
                {
                    if(Register_status[inst->rs] =="" &&Register_status[inst->rt] =="" )
                        return 1;
                    else
                        return 0;
                }

            }

        }
    }
        // if(!branch)
        // {
        //     PC++;
        // }
        // cycle++;
        // Register_print();
        // Data_print();

    
    // outFile_simu<<"--------------------"<<endl;
    // outFile_simu<<"Cycle:"<<cycle + 1<<"\t"<<Inst_set[PC]->address<<"\t";
    // //Inst_print(Inst_set[PC], outFile_simu); 
    // outFile_simu<<endl;
    // Register_print();
    // Data_print();
}

void Execute_print(Inst* inst)
{

}

void Register_print()
{
    outFile_simu<<"Registers";
  //  outFile_simu<<"R00:";
    for(int i = 0; i < 32; i++)
    {
        if(i % 8 == 0 )
        {
            if(i < 10)
                outFile_simu<<endl<<"R0"<<i<<":"; 
            else
                outFile_simu<<endl<<"R"<<i<<":"; 
        }
     
        outFile_simu<<"\t"<<Register[i];
    }
    outFile_simu<<endl<<endl;
}

void Data_print()
{
    outFile_simu<<"Data"<<endl;
    int Data_count = 0;
    int start = 0;
    
    for(start = mem_start; start <= mem_end; start += 4)
    {
        if(Data_count % 8 == 0)
            outFile_simu<<start<<":";
        outFile_simu<<"\t"<<mem_map[start];

        if(Data_count % 8 == 7)
            outFile_simu<<endl;
        Data_count++;
    }
   // outFile_simu<<endl;
}

void Inst_print(Inst* inst, ofstream &outFile)
{
    string assem = "";
    if(inst->Itpye == "R")
    {
        if(inst->op == "BREAK")
        {
            assem = assem + "BREAK";
           // outFile<<"BREAK"<<endl;
        }
        else if(inst->op == "SLL"||inst->op == "SRL")
        {

            assem = assem + inst->op+ "\tR" + to_string(inst->rd)+", R" + to_string(inst->rt) + ", #" + to_string(inst->shamt);

          //  outFile<<inst->op<<" "<<"R"<<inst->rd<<", R"<<inst->rt<<", #"<<inst->shamt<<endl;

        }
        else
        {
            assem = assem + inst->op+ "\tR" + to_string(inst->rd)+", R" + to_string(inst->rs) + ", R" + to_string(inst->rt) ;

            //outFile<<inst->op<<" "<<"R"<<inst->rd<<", R"<<inst->rs<<", R"<<inst->rt<<endl;
        }

     
    }
    else if(inst->Itpye == "J")
    {

        if(inst->op == "J")
        {
                        
            assem = assem + "J\t#" + to_string(inst->immd*4);

         //   outFile<<"J #"<<inst->immd*4<<endl;
        }

       
    }
    else
    {
        if(inst->op == "ADD" ||inst->op == "SUB" || inst->op == "MUL"||inst->op == "AND"||inst->op == "NOR"||inst->op == "SLT"||inst->op == "BEQ")
        {
            if(inst->op == "BEQ")
            {    
            assem = assem + inst->op+ "\tR" + to_string(inst->rs)+", R" + to_string(inst->rt) + ", #" + to_string(inst->immd*4) ;

              //  outFile<<inst->op<<" "<<"R"<<inst->rs<<", R"<<inst->rt<<", #"<<inst->immd*4<<endl;
            }
            else
            {
                assem = assem + inst->op+ "\tR" + to_string(inst->rt)+", R" + to_string(inst->rs) + ", #" + to_string(inst->immd) ;

              //  outFile<<inst->op<<" "<<"R"<<inst->rt<<", R"<<inst->rs<<", #"<<inst->immd<<endl;

            }    


        }    
        else if(inst->op == "BGTZ"||inst->op =="BLTZ" )
        {            
            assem = assem + inst->op+ "\tR" + to_string(inst->rs)+", #" + to_string(inst->immd*4) ;

         //   outFile<<inst->op<<" "<<"R"<<inst->rs<<", #"<<inst->immd*4<<endl;

        }
        else if(inst->op == "LW" || inst->op == "SW")
        {
            assem = assem + inst->op+ "\tR" + to_string(inst->rt)+", " + to_string(inst->immd) + "(R" + to_string(inst->rs) + ")" ;

            //outFile<<inst->op<<" "<<"R"<<inst->rt<<", "<<inst->immd<<"(R"<<inst->rs<<")"<<endl;


        }
    }
    //outFile<<inst->assmble<<endl;
    inst->assmble =  assem ;
    outFile<<inst->assmble<<endl;

    
}


int string_32b_To_int(string input)
{
    int Max = 1<<31;
    int res;
    string unsiged_int = input.substr(1, 31);
    res = stoi(unsiged_int, 0, 2);

    if(input[0] == '1')
    {
        res += Max;             // 补上符号位
    }
    return res;
}