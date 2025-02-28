#ifndef ARDUINO
#include "Arduino.h"
#include <cstring>
#include <deque>
#include <locale>
#include <thread>
#include <chrono>

int main(int argc, const char* argv[]){
	
	if(argc==2 && ((strcmp(argv[1],"--help")==0) || (strcmp(argv[1],"-h")==0))){
		std::cerr<<"Usage: "<<argv[0]<<" [com_port=/dev/ttyACM0] [verbosity=0]"<<std::endl;
		return 0;
	}
	
	std::cout<<"making Arduino class"<<std::endl;
	Arduino arduino;
	
	std::string dev = "/dev/ttyACM0";
	int baud = 19200;
	int verbosity = 1;
	
	if(argc>1){
		dev = argv[1];
	}
	if(argc>2){
		verbosity = std::atoi(argv[2]);
		if(verbosity==0 && strcmp(argv[2],"0")!=0){
			std::cerr<<"Error parsing verbosity '"<<argv[2]<<"'"<<std::endl;
		}
	}
	
	std::cout<<"Initialising"<<std::endl;
	bool ok = arduino.Configure(dev,baud,verbosity);
	if(!ok){
		std::cerr<<"Error configuring"<<std::endl;
		return 2;
	}
	
	ok = arduino.Connect();
	if(!ok){
		std::cerr<<"Error connecting"<<std::endl;
		return 3;
	}
	
	//std::deque<std::string> last_cmds;
	//int last_idx=0;
	
	bool quit=false;
	do {
		
		// get next command from user
		std::cout<<"> Enter next command:"<<std::endl;
		std::string cmd;
		getline(std::cin,cmd);
		std::cout<<"Processing command '"<<cmd<<"'"<<std::endl;
		
		// should be a key-value pair, value is optional depending on command
		// e.g. '275_A 1' will enable 275 LED, while 'BEEP' will emit a beep.
		std::stringstream ss(cmd);
		std::string key, val;
		ss >> key >> val;
		//std::cout<<"key: '"<<key<<"', val: '"<<val<<"'"<<std::endl;
		
		// cast to lowercase so we don't have to bother with case insensitive matches
		for(char& c : key) c=std::tolower(c);
		for(char& c : val) c=std::tolower(c);
		
		// similarly be relaxed about value syntax
		if(val=="1" || val=="on" || val=="true") val="ENABLE";
		if(val=="0" || val=="off" || val=="false") val="DISABLE";
		
		if(key=="quit") break;
		
		// forward to arduino
		std::string msg = key+" "+val;
		std::string resp;
		ok = arduino.SendAndReceive(msg, resp);
		std::cout<<resp<<std::endl;
		
		if(!ok) continue;
		
		//if(key!="up" && key!="down") last_cmds.push_front(cmd);
		
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
	} while(!quit);
	
	std::string resp;
	ok = arduino.SendAndReceive("OFF", resp);
	std::cout<<resp<<std::endl;
	ok = ok && arduino.Disconnect();
	
	if(!ok){
		std::cerr<<"Finalisation error!"<<std::endl;
		return 10;
	}
	
	return 0;
}

#endif



// recognise arrow keys - this will requrie waaaay more work with the ncurses library (or kbhit...)
// otherwise we can't access keypresses without enter.
/*
if(int(key[0])==27 && int(key[1])==91){
	// escape code for arrow keys
	if(int(key[2])==65) key="up";
	else if(int(key[2])==66) key="down";
	else {
		std::cerr<<"unrecognised escape sequence"<<std::endl;
		continue;
	}
	std::cout<<key<<" pressed!"<<std::endl;
	if(key=="up"){
		if(!last_cmds.size()) continue;
		if(last_idx<(last_cmds.size()-1)) ++last_idx;
		for(char& c : last_cmds.at(last_idx)) ungetc(int(c), stdin);
		continue;
	}
	if(key=="down"){
		if(!last_cmds.size()) continue;
		if(last_idx>0) --last_idx;
		for(char& c : last_cmds.at(last_idx)) ungetc(int(c), stdin);
		continue;
	}
}
*/
