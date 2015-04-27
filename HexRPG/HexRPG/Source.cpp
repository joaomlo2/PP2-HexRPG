#include <stdio.h>
#include <stdlib.h>

typedef struct hexagono
{                                       /*																			EX:Se x=5 e y=5:		 ___		*/
	struct hexagono*NO;					/*	     2(N)		    	-Se for por 1, as coordenadas serão (x-1,y-1)	(5-1,5-1)=(4,4)		 ___/5,3\___	*/
	struct hexagono*N;					/* 		 ___		    	-"	"	"	2, "  "			  "		(x,y-2)		(5,5-2)=(5,3)		/4,4\___/6,4\	*/
	struct hexagono*NE;					/* 1(NO)/x,y\3(NE)	    	-"	"	"	3, "  "           "		(x+1,y-1)	(5+1,5-1)=(6,4)		\___/5,5\___/	*/
	struct hexagono*SO;					/* 6(SO)\___/4(SE)			-"	"	"	4, "  "           "		(x+1,y+1)	(5+1,5+1)=(6,6)		/4,6\___/6,6\	*/
	struct hexagono*S;					/*		 5(S)				-"	"	"	5, "  "           "		(x,y+2)		(5,5+2)=(5,7)		\___/5,7\___/	*/
	struct hexagono*SE;					/*							-"	"	"	6, "  "           "     (x-1,y+1)	(5-1,5+1)=(4,6)			\___/		*/
	int x;
	int y;
}*HEX;

void Mostrar_Celula(HEX Mapa,int px,int py)
{
	HEX aux = Mapa;
	if (aux == NULL)
		printf("Essa Celula nao existe!");
	else
	{
		if (aux->N != NULL)
			printf("    %d,%d\n", aux->N->x, aux->N->y);
		else
			printf("    NULL\n");
		if (aux->NO != NULL)
			printf("%d,%d	", aux->NO->x, aux->NO->y);
		else
			printf("NULL	");
		if (aux->NE != NULL)
			printf("%d,%d\n", aux->NE->x, aux->NE->y);
		else
			printf("NULL\n");
		printf("    %d,%d    	\n", aux->x, aux->y);
		if (aux->SE != NULL)
			printf("%d,%d     ",aux->SE->x, aux->SE->y);
		else
			printf("NULL	");
		if (aux->SO != NULL)
			printf("%d,%d\n", aux->SO->x, aux->SO->y);
		else
			printf("NULL\n");
		if (aux->S != NULL)
			printf("    %d,%d\n", aux->S->x, aux->S->y);
		else
			printf("    NULL\n");
	}
}

int Vizinhança(HEX apt)
{
	int contador = 0;
	if (apt->NO != NULL)
		contador++;
	if (apt->N != NULL)
		contador++;
	if (apt->NE != NULL)
		contador++;
	if (apt->S != NULL)
		contador++;
	if (apt->SE != NULL)
		contador++;
	if (apt->SO != NULL)
		contador++;
	return contador;
}

HEX Navegar_Ate_Celula(HEX Mapa, int X, int Y)
{

}

HEX Add_Hex(HEX apt,int dir)
{
	HEX aux = apt;
	if (dir == 1)
	{
		if (aux->NO == NULL)
		{
			aux == aux->NO;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x=(apt->x) - 1;
			aux->y=(apt->y) - 1;

		}
	}
}

HEX Add_Camada(HEX Mapa)
{
	HEX aux = Mapa;
	if (aux == NULL)
	{
		aux = (HEX)malloc(sizeof(struct hexagono));
		aux->N = NULL;
		aux->NE = NULL;
		aux->NO = NULL;
		aux->S = NULL;
		aux->SE = NULL;
		aux->SO = NULL;
		aux->x = 0;
		aux->y = 0;
	}
	else
	{

	}
}



void main()
{
	HEX Mapa = NULL;
	Mapa = Add_Hex(Mapa, 0,0);
	printf("x:%d\ny:%d\n",Mapa->x,Mapa->y);
	Mostrar_Celula(Mapa,Mapa->x,Mapa->y);
	system("pause");
	free(Mapa);
}