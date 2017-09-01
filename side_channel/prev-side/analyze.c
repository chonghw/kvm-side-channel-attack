#include <stdio.h>

void input(FILE * in);
void process();
void compare();
void getresult();
void output(FILE * out);

char data[100];
int data_len;
int state, check, len;
char result[1000000], tmpc;
char ans[10000];
int cnt, sum, anslen;
int avg[1000000];

int main(int argc,char **argv)
{
	FILE * in = fopen(argv[1], "r");
	FILE * in2 = fopen(argv[2], "r");
	FILE * out = fopen("result.txt","w");


	state = 0; 

	fscanf(in2, "%s", ans+1);
	anslen = strlen(ans+1);

	while(!feof(in))
	{
		check =0;
		data_len = 0;

		if(tmpc !=0) data[++data_len]= tmpc;

		input(in);
		
		//printf("%d %s\n",data_len, data+1);

		if(state && !check) process();
		else if(!state && check) output(out);
	}

	getresult(out);

	fclose(in);
	fclose(in2);
	fclose(out);
	return 0;
}

void input(FILE * in)
{
	char c, prevc;
	int numcheck=0, charcheck=0;

	tmpc = 0;

	while(!feof(in))
	{
		fscanf(in,"%c\n", &c);
		
		if(c != '.')
		{
			if(data_len >= 2 && (prevc=='.' || (numcheck && c == '#') || (charcheck && (c !='.' && c!='#'))) )break;

			data[++data_len] = c;

			if(data[data_len] == '#')
			{
				charcheck = 1;
				check=1;
			}
			else numcheck = 1;
		}
		prevc = c;
	}
	tmpc = c;
	if(charcheck)state = !state;

	data[data_len+1]='\0';
}

void process()
{
	int i;
	int count[2]={0,0};
	for(i=1; i<=data_len; i++)
	{
		if(data[i]=='?')continue;
		count[data[i]-'0']++;
	}

	if(count[0] > count[1])result[++len]='0';
	else if(count[0] < count[1])result[++len]='1';
	else result[++len]='?';
}

void compare()
{
	int i, tmpcnt=0;
	cnt++;

	if(len != anslen)return;

	for(i=1; i<=anslen; i++)
	{
		if(result[i] == ans[i])tmpcnt++;
	}

	sum += tmpcnt;

	avg[cnt]=tmpcnt;
}

void output(FILE * out)
{
	result[len+1]=0;
	printf("%s\n", result+1);
	fprintf(out,"%s\n", result+1);

	compare();

	len =0;
}

void getresult(FILE * out)
{
	int i;
	double tmpavg=0, sd=0;

	for(i=1; i<=cnt; i++)
		tmpavg += 1 - (double)avg[i] / anslen;
	
	tmpavg /= cnt;

	for(i=1; i<=cnt; i++)
		sd += (1- (double)avg[i] / anslen - tmpavg)*(1- (double)avg[i] / anslen - tmpavg);
	sd /= cnt;

	printf("Average of Error rate : %.3lf\n", tmpavg);
	printf("Standard deviation : %.3lf\n", sqrt(sd));

	fprintf(out,"Average of Error rate : %.3lf\n", tmpavg);
	fprintf(out,"Standard deviation : %.3lf\n", sqrt(sd));
}

