#include<iostream>
#include<string>
using namespace std;

void bitstuffing()
{
   string data,stuffed_data="";
   int i, count=0;
   
   cout << "Enter binary data:" << endl;
   cin >> data;
   
   for(i=0;i<data.length();i++)
   {
     if(data[i]=='1')
     {
       count ++;
       stuffed_data +=data[i];
       if(count==5)
       {
         stuffed_data +='0';
         count=0;
       }
     }
     else
     {
       count=0;
       stuffed_data +=data[i];
     }
   }
    cout <<"stuffed data: 01111110"<<stuffed_data<<"01111110" << endl;
    
}   

void bytestuffing()
{
  string data,stuffed_data="";
  int i;
  cout << "Enter character data:" << endl;
  cin >> data;
  
  for(i=0;i<data.length();i++)
  {
    if(data[i]=='F' ||data[i]=='E')
    {
      stuffed_data +='E';
    }
    stuffed_data +=data[i];
  }
  cout << "stuffed data:F"<<stuffed_data<<'F' << endl;
}                        
int main()
{
  cout << "HDLC frame implementation:\n" << endl;
  cout << "Bit stuffing:\n" << endl;
  bitstuffing();
  cout << "\n----------------------------\n" << endl;
  cout << "Byte stuffing:\n" << endl;
  bytestuffing();
  return 0;
}
