/*

Resources used:
	1. https://youtu.be/NbSee-XM7WA (great tutorial by javidx9)

*/

#include <iostream>
#include <Windows.h>
#include <string>
#include <chrono>

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_3 1.0471975511965977461533
#define M_PI_3_2 4.71238898038468985769


int main() {

	int nScreenHeight = 40;
	int nScreenWidth = 120;

	int nMapHeight = 16;
	int nMapWidth = 16;

	float fSpeed = 5.0f;

	float fPlayerX = 8.0f;
	float fPlayerY = 8.0f;
	float fPlayerA = M_PI_2;
	float fFOV = M_PI_3;
	float fDepth = 16.0f;

	float fDistanceToProjection = ((float)nScreenWidth / 2) / tanf(fFOV / 2);

	wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	std::string map;

	map += "################";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..###...###...#";
	map += "#..............#";
	map += "#..#..###......#";
	map += "#..#...........#";
	map += "#..#......##...#";
	map += "#..............#";
	map += "################";

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (true) {

		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		// Handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA += (fSpeed * 0.75f) * fElapsedTime;

		// Handle CW Rotation
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

		for (int col = 0; col < nScreenWidth; col += 1) {

			// ray angle for this column in the screen
			float fRayAngle = (fPlayerA + fFOV / 2) - (fFOV / nScreenWidth * col);
			float fCosRayAngle = cosf(fRayAngle);
			float fSinRayAngle = sinf(fRayAngle);
			float fACosRayAngle = 1 / fCosRayAngle;
			float fASinRayAngle = 1 / fSinRayAngle;

			float fRayLengthX;
			int nStepX;
			float fRayLengthY;
			int nStepY;

			int nMapCheckX = (int)fPlayerX;
			int nMapCheckY = (int)fPlayerY;

			// X increment +1
			if (fCosRayAngle > 0) {
				nStepX = 1;
				fRayLengthX = (nMapCheckX + 1 - fPlayerX) * fabs(fACosRayAngle);
			}
			// X increment -1
			else {
				nStepX = -1;
				fRayLengthX = (fPlayerX - nMapCheckX) * fabs(fACosRayAngle);
			}
			// Y increment +1
			if (fSinRayAngle > 0) {
				nStepY = 1;
				fRayLengthY = (nMapCheckY + 1 - fPlayerY) * fabs(fASinRayAngle);
			}
			// Y increment -1
			else {
				nStepY = -1;
				fRayLengthY = (fPlayerY - nMapCheckY) * fabs(fASinRayAngle);
			}

			boolean bHitWall = false;
			float fDistanceToWall = 0.0f;

			while (!bHitWall && fDistanceToWall < fDepth) {

				if (fRayLengthX < fRayLengthY) {
					nMapCheckX += nStepX;
					fDistanceToWall = fRayLengthX;
					fRayLengthX += fabs(fACosRayAngle);
				}
				else {
					nMapCheckY += nStepY;
					fDistanceToWall = fRayLengthY;
					fRayLengthY += fabs(fASinRayAngle);
				}

				if (nMapCheckX >= 0 && nMapCheckX < nMapWidth && nMapCheckY >= 0 && nMapCheckY < nMapHeight) {

					if (map[nMapCheckY * nMapWidth + nMapCheckX] == '#') {
						bHitWall = true;
					}
				}
			}

			float fHitX = fPlayerX + fCosRayAngle * fDistanceToWall;
			float fHitY = fPlayerY + fSinRayAngle * fDistanceToWall;

			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// Shader walls based on distance
			short nShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	// Very close	
			else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
			else if (fDistanceToWall < fDepth)				nShade = 0x2591;
			else											nShade = ' ';		// Too far away

			for (int row = 0; row < nScreenHeight; row += 1)
			{
				// Each Row
				if (row <= nCeiling)
					screen[row * nScreenWidth + col] = ' ';
				else if (row > nCeiling && row <= nFloor)
					screen[row * nScreenWidth + col] = nShade;
				else // Floor
				{
					// Shade floor based on distance
					float b = 1.0f - (((float)row - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[row * nScreenWidth + col] = nShade;
				}
			}
		}

		// Display Map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	delete[] screen;
	return 0;
}
