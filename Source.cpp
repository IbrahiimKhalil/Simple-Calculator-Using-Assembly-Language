#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <emscripten.h>

using namespace std;
vector<string> UnKnowns;
class Terms
{
  public:
    string unknown;
    float coeff;
    string Sign;
    string Term;
    Terms(string Trm, string sign)
    {
        Term = Trm;
        Sign = sign;
        int i;
        for (i = 0; i < Term.length(); i++)
        {
            if (isalpha(Term[i]))
                break;
        }
        coeff = atoi(Term.substr(0, i).c_str());
        if (i == 0)
            coeff = 1;
        unknown = Term.substr(i, 1);
        int flag = 0;
        if (unknown != "")
        {
            for (int i = 0; i < UnKnowns.size(); i++)
            {
                if (UnKnowns[i] == unknown)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
                UnKnowns.push_back(unknown);
        }
    }
};
class EquationSide
{
  public:
    vector<Terms> TmpTerms;
    vector<Terms> CurTerms;
    void SetSide(string side)
    {
        string CurTerm = "";
        string TmpOper = "+";
        int i;
        for (i = 0; i < side.length(); i++)
        {
            if (side.substr(i, 1) == "+" || side.substr(i, 1) == "-")
            {
                if (CurTerm != "")
                {
                    cout << TmpOper << CurTerm << endl;
                    TmpTerms.push_back(Terms(CurTerm, TmpOper));
                }
                TmpOper = side.substr(i, 1);
                CurTerm = "";
            }
            else
                CurTerm = CurTerm + side.substr(i, 1);
        }
        if (i == side.length())
        {
            if (CurTerm != "")
            {
                cout << TmpOper << CurTerm << endl;
                TmpTerms.push_back(Terms(CurTerm, TmpOper));
            }
        }
    }
    void Reduce()
    {
        int flag;
        for (int j = 0; j < TmpTerms.size(); j++)
        {
            flag = 0;
            for (int i = 0; i < CurTerms.size(); i++)
            {
                if (CurTerms[i].unknown == TmpTerms[j].unknown)
                {
                    float c = CurTerms[i].coeff;
                    float t = TmpTerms[j].coeff;
                    if (CurTerms[i].Sign == "-")
                        c = -1 * CurTerms[i].coeff;
                    if (TmpTerms[j].Sign == "-")
                        t = -1 * TmpTerms[j].coeff;
                    CurTerms[i].coeff = c + t;
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
                CurTerms.push_back(Terms(TmpTerms[j].Term, TmpTerms[j].Sign));
        }
    }
};
class Equation
{
  public:
    EquationSide TmpLHS, TmpRHS, LHS, RHS;
    Equation(string Eqn)
    {
        int EqualIndex = Eqn.find("=");
        cout << "New Equation : " << endl;
        cout << "LHS : " << Eqn.substr(0, EqualIndex) << endl;
        cout << "RHS : " << Eqn.substr(EqualIndex + 1) << endl;
        cout << "LHS Terms : " << endl;
        TmpLHS.SetSide(Eqn.substr(0, EqualIndex));
        cout << "RHS Terms : " << endl;
        TmpRHS.SetSide(Eqn.substr(EqualIndex + 1));
        string Oper = "";
        for (int i = 0; i < TmpLHS.TmpTerms.size(); i++)
        {
            if (TmpLHS.TmpTerms[i].unknown == "")
            {
                if (TmpLHS.TmpTerms[i].Sign == "-")
                    Oper = "+";
                else if (TmpLHS.TmpTerms[i].Sign == "+")
                    Oper = "-";
                RHS.TmpTerms.push_back(Terms(TmpLHS.TmpTerms[i].Term, Oper));
            }
            else
                LHS.TmpTerms.push_back(Terms(TmpLHS.TmpTerms[i].Term, TmpLHS.TmpTerms[i].Sign));
        }
        for (int k = 0; k < TmpRHS.TmpTerms.size(); k++)
        {
            if (TmpRHS.TmpTerms[k].unknown != "")
            {
                if (TmpRHS.TmpTerms[k].Sign == "-")
                    Oper = "+";
                else if (TmpRHS.TmpTerms[k].Sign == "+")
                    Oper = "-";
                LHS.TmpTerms.push_back(Terms(TmpRHS.TmpTerms[k].Term, Oper));
            }
            else
            {
                RHS.TmpTerms.push_back(Terms(TmpRHS.TmpTerms[k].Term, TmpRHS.TmpTerms[k].Sign));
            }
        }
        LHS.Reduce();
        RHS.Reduce();
    }
};
vector<Equation> Equations;

vector<vector<float> > CoeffsMatrix;
vector<float> RHSMatrix;

class Utility
{
  public:
    static void SetMatrix()
    {
        for (int i = 0; i < Equations.size(); i++)
        {
            vector<float> coef;
            float c;
            for (int j = 0; j < UnKnowns.size(); j++)
            {
                int flag = 0;
                for (int k = 0; k < Equations[i].LHS.CurTerms.size(); k++)
                {
                    if (UnKnowns[j] == Equations[i].LHS.CurTerms[k].unknown)
                    {
                        c = Equations[i].LHS.CurTerms[k].coeff;
                        if (Equations[i].LHS.CurTerms[k].Sign == "-")
                            c = -1 * Equations[i].LHS.CurTerms[k].coeff;
                        coef.push_back(c);
                        flag = 1;
                        break;
                    }
                }
                if (flag == 0)
                    coef.push_back(0);
            }
            CoeffsMatrix.push_back(coef);
            c = Equations[i].RHS.CurTerms[0].coeff;
            if (Equations[i].RHS.CurTerms[0].Sign == "-")
                c = -1 * Equations[i].RHS.CurTerms[0].coeff;
            RHSMatrix.push_back(c);
        }
    }
    static vector<vector<float> > GetVariableMatrix(int var, vector<vector<float> > CoeffMatrix, vector<float> RhsMatrix)
    {
        for (int i = 0; i < CoeffMatrix.size(); i++)
        {
            CoeffMatrix[i][var] = RhsMatrix[i];
        }
        return CoeffMatrix;
    }
    static void DisplayEquation()
    {
        for (int i = 0; i < Equations.size(); i++)
        {
            cout << endl;
            for (int j = 0; j < Equations[i].LHS.CurTerms.size(); j++)
            {
                if (j != 0 || Equations[i].LHS.CurTerms[j].Sign == "-")
                    cout << Equations[i].LHS.CurTerms[j].Sign;
                cout << Equations[i].LHS.CurTerms[j].coeff << Equations[i].LHS.CurTerms[j].unknown;
            }
            cout << "=";
            for (int k = 0; k < Equations[i].RHS.CurTerms.size(); k++)
            {
                if (k != 0 || Equations[i].RHS.CurTerms[k].Sign == "-")
                    cout << Equations[i].RHS.CurTerms[k].Sign;
                cout << Equations[i].RHS.CurTerms[k].coeff << Equations[i].RHS.CurTerms[k].unknown;
            }
            cout << endl;
        }
    }
    static void DisplayMatrix(vector<vector<float> > mat)
    {
        for (int i = 0; i < mat.size(); i++)
        {
            for (int j = 0; j < mat[i].size(); j++)
            {
                cout << mat[i][j] << " ";
            }
            cout << endl;
        }
    }
    static int CalculateDeterminant(vector<vector<float> > Matrix)
    {
        float det = 0;
        if (Matrix.size() == 1)
        {
            return Matrix[0][0];
        }
        else if (Matrix.size() == 2)
        {
            det = (Matrix[0][0] * Matrix[1][1] - Matrix[0][1] * Matrix[1][0]);
            return det;
        }
        else
        {
            for (int p = 0; p < Matrix[0].size(); p++)
            {
                vector<vector<float> > TmpMatrix;
                for (int i = 1; i < Matrix.size(); i++)
                {
                    vector<float> TmpRow;
                    for (int j = 0; j < Matrix[i].size(); j++)
                    {
                        if (j != p)
                        {
                            TmpRow.push_back(Matrix[i][j]);
                        }
                    }
                    if (TmpRow.size() > 0)
                        TmpMatrix.push_back(TmpRow);
                }
                det = det + Matrix[0][p] * pow(-1, p) * CalculateDeterminant(TmpMatrix);
            }
            return det;
        }
    }
};
extern "C"
{
    EMSCRIPTEN_KEEPALIVE
    void PassEquations(char *Eqn)
    {
        //this function is written in c++ to take the equations as string then parse each equation 
        //and get the solution for the system of linear equations
        string Eqnx(Eqn); //first we convert the passed argument from char * to std::string
        vector<string> EquationText; // declare the vector which we will add the equations to it
        //after splitting the string using the ";" character which separates between the equations
        while (true)
        {
            int SeparatorIndex = Eqnx.find(";"); //get the index of the first ";" character
            if (SeparatorIndex != -1) //if the index isn't equal to -1 which means we found the ";" character
            {
                EquationText.push_back(Eqnx.substr(0, SeparatorIndex)); //then add to the vector the equation 
                //by taking the substring of Eqnx from 0 to separator index 
                Eqnx = Eqnx.substr(SeparatorIndex + 1); // add the rest of the string to same variable 
                //to work on it again to get the next equation
            }
            else
                break; //then if the index is equal to -1 we will break 
                //as this means that we already added all the equations in the string to our vector
        }
        for (int i = 0; i < EquationText.size(); i++) //then we will iterate on each equation
        {
            Equations.push_back(Equation(EquationText[i]));//create instance from the equation class and pass to it 
            //the equation string then add the object to Equations vector
        }
        Utility::DisplayEquation();//then we will call this function to display the equations on the console log
        Utility::SetMatrix();//call this function to form our Coefficient matrix
        float CoefMatrixDet = Utility::CalculateDeterminant(CoeffsMatrix);//then get the determinant of this 
        //Coeffiecient matrix using the CalculateDeterminant function
        vector<float> VariableDeterminant;//declare the vector where we will store the determinants of the variable matrices
        for (int i = 0; i < UnKnowns.size(); i++)
        {
            //we will iterate on each unknown to get its variable matrix and calculate its determinant
            VariableDeterminant.push_back(Utility::CalculateDeterminant(Utility::GetVariableMatrix(i, CoeffsMatrix, RHSMatrix)));
            //we will add to the vector the determinant of the return value of the function GetVariableMatrix 
            //which takes the order of the unknown , the coefficient matrix, right hand side matrix and returns 
            //the variable matrix for each unknown
        }
        string ResultsStr ="";//declare the string where we will store the results and pass it to the JavaScript function
        for (int j = 0; j < VariableDeterminant.size(); j++)
        {
            cout << endl
                 << endl;
            cout << UnKnowns[j] << " = " << VariableDeterminant[j] / CoefMatrixDet;
            cout << endl
                 << endl;
            //we will iterate on each value in the vector which contains the determinant 
            //of the variable matrix for each unknown
            int r = (int)(VariableDeterminant[j] / CoefMatrixDet);//convert the value of determinant from float to int
            ResultsStr += UnKnowns[j] + " = " + to_string(r) + ";";//form the string which contains the result
        }
        string script = "SolutionPanel('" + ResultsStr + "')";//this is the JavaScript script that calls the SolutionPanel
        //function and pass to it the string which contains results
        const char *s = script.c_str(); //convert this string which contains the script to char * as this is the expected
        //type of the argument for the emscripten_run_script function
        emscripten_run_script(s);//Call the function which execute our script
    }
}
EMSCRIPTEN_KEEPALIVE
int main(int argc, char const *argv[])
{
    int Count = atoi(argv[1]);
    if (Count < 2)
        return 0;
    for (int i = 0; i < Count; i++)
    {
        string Eqn = "";
        cout << "Enter Equation No : " << (i + 1) << " ";
        cin >> Eqn;
        Equations.push_back(Equation(Eqn));
    }
    Utility::DisplayEquation();
    Utility::SetMatrix();
    float CoefMatrixDet = Utility::CalculateDeterminant(CoeffsMatrix);
    vector<float> VariableDeterminant;
    for (int i = 0; i < UnKnowns.size(); i++)
    {
        VariableDeterminant.push_back(Utility::CalculateDeterminant(Utility::GetVariableMatrix(i, CoeffsMatrix, RHSMatrix)));
    }
    cout << CoefMatrixDet;
    for (int j = 0; j < VariableDeterminant.size(); j++)
    {
        cout << endl
             << endl;
        cout << UnKnowns[j] << " = " << VariableDeterminant[j] / CoefMatrixDet;
        cout << endl
             << endl;
    }
}
