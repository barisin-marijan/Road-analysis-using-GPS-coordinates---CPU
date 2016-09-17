#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

typedef struct point
{
	double coordinates[3];
};

typedef struct line
{
	double slope, point;
};

void load_numbers(char* string, struct point &coordinate)
{
	int i = 0, j, k, l;
	double broj;

	for (j = 0; j < 3; j++)
		coordinate.coordinates[j] = 0;

	for (j = 0; j < 3; j++)
	{
		while (string[i] != '.' && isdigit(string[i]))
		{
			coordinate.coordinates[j] = coordinate.coordinates[j] * 10 + string[i] - '0';
			i++;
		}
		i++;

		k = 1;
		while (string[i] != ',' && isdigit(string[i]) && i < strlen(string))
		{
			broj = string[i] - '0';
			for (l = 0; l < k; l++)
				broj = broj / (double)10;
			coordinate.coordinates[j] = coordinate.coordinates[j] + broj;
			i++;
			k++;
		}
		i++;
	}
}

int load_data(char *file_name, struct point *field)
{
	FILE* f;
	char *string, character;
	struct point helper;
	int i = 0;

	f = fopen(file_name, "r");
	if (!f)
	{
		printf("POGRESKA PRI OTVARANJU DATOTEKE!");
		exit(1);
	}
	string = (char*)malloc(50 * sizeof(char));

	fscanf(f, "%s ", string);
	while (strncmp(string, "<altitudeMode>clampedToGround</altitudeMode>", strlen("<altitudeMode>clampedToGround</altitudeMode>")) != 0)
		fscanf(f, "%s ", string);

	fscanf(f, "%c", &character);
	while (character != '>')
		character = fgetc(f);

	fscanf(f, "%s", string);
	load_numbers(string, helper);
	field[0].coordinates[0] = helper.coordinates[0];
	field[0].coordinates[1] = helper.coordinates[1];
	field[0].coordinates[2] = helper.coordinates[2];
	i++;

	while (strncmp(string, "</coordinates>", strlen("</coordinates>")) != 0)
	{
		fscanf(f, "%s", string);
		load_numbers(string, helper);
		field[i].coordinates[0] = helper.coordinates[0];
		field[i].coordinates[1] = helper.coordinates[1];
		field[i].coordinates[2] = helper.coordinates[2];
		i++;
	}

	fclose(f);
	return i;
}

void ispis(struct point *field, int n)
{
	int i;
	for (i = 0; i < n; i++)
		printf("\n%lf,%lf,%lf", field[i].coordinates[0], field[i].coordinates[1], field[i].coordinates[2]);
}

void find_tangent(struct point a, struct point b, struct line &solution)
{
	double slope, point;
	slope = -(b.coordinates[0] - a.coordinates[0]) / (b.coordinates[1] - a.coordinates[1]);
	point = -slope * (a.coordinates[0] + b.coordinates[0]) / (double)2 + (a.coordinates[1] + b.coordinates[1]) / (double)2;
	solution.slope = slope;
	solution.point = point;
}

void find_midpoint(struct point a, struct point b, struct point &solution)
{
	solution.coordinates[0] = (a.coordinates[0] + b.coordinates[0]) / (double)2;
	solution.coordinates[1] = (a.coordinates[1] + b.coordinates[1]) / (double)2;
	solution.coordinates[2] = (a.coordinates[2] + b.coordinates[2]) / (double)2;
}

int find_intersection(struct line a, struct line b, struct point &solution)
{
	double x, y;
	if (abs(a.slope - b.slope) < 0.0000001)
		return -1;
	else
	{
		x = (b.point - a.point) / (a.slope - b.slope);
		y = a.slope * x + a.point;
		solution.coordinates[0] = x;
		solution.coordinates[1] = y;
		solution.coordinates[2] = 0;
		return 1;
	}
}

double find_distance(struct point a, struct point b)
{
	double d;
	d = sqrt((b.coordinates[0] - a.coordinates[0])*(b.coordinates[0] - a.coordinates[0]) + (b.coordinates[1] - a.coordinates[1])*(b.coordinates[1] - a.coordinates[1]));
	return d;
}

char determine_road(struct point a, struct point b, struct point c)
{
	double distance1, distance2, condition;
	struct point intersection, midpoint1, midpoint2;
	struct line tangent1, tangent2;
	int flag;

	find_tangent(a, b, tangent1);
	find_tangent(b, c, tangent2);
	flag = find_intersection(tangent1, tangent2, intersection);
	if (flag == -1)
		return 'p';

	find_midpoint(a, b, midpoint1);
	find_midpoint(b, c, midpoint2);
	distance1 = find_distance(intersection, midpoint1);
	distance2 = find_distance(intersection, midpoint2);
	condition = distance1 - distance2;

	if (abs(condition) < 0.00000001)
		return 'r';
	else
		return 'l';
}

void write_to_file(char* name_of_file, char *data, struct point *field, char *results, int size)
{
	FILE *f;
	int i;
	f = fopen(name_of_file, "w");

	if (!f)
	{
		printf("POGRESKA PRI OTVARANJU DATOTEKE!");
		exit(1);
	}

	fprintf(f, "Rezultati za %s", data);
	for (i = 0; i < size; i++)
		fprintf(f, "\n%lf %lf %lf %c", field[i].coordinates[0], field[i].coordinates[1], field[i].coordinates[2], results[i]);
	fclose(f);
}

int main()
{
	struct point *field;
	char *results;
	int i, n;
	clock_t begin, end;
	double time_spent;

	begin = clock();

	field = (struct point*)malloc(20000 * sizeof(struct point));
	results = (char*)malloc(20000 * sizeof(char));

	n = load_data("V1_S_V_24_10_2012.kml", field) - 1;
	for (i = 1; i < n - 1; i++)
		results[i] = determine_road(field[i - 1], field[i], field[i + 1]);
	results[0] = 'N';
	results[n - 1] = 'N';
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("DONE!");
	printf("\nTime: %lf", time_spent);
	write_to_file("V1.2_Records.txt", "V1_S_V_24_10_2012_V.2.kml", field, results, n);

	getchar();
	return 0;
}