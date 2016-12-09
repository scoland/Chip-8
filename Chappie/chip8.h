#pragma once
#pragma warning(disable:4996)
class Chip8
{
public:
	Chip8();
	~Chip8();

	unsigned char key[16];
	unsigned char gfx[64 * 32];

	void initialize();
	void emulateCycle();
	bool loadFile(const char * filename);
	void debugRender();

	bool drawFlag;

private:
	unsigned short opcode; // Current opcode
	unsigned char memory[4096]; // 4K memory
	unsigned char V[16]; // 16 CPU registers V0 - VF

	unsigned short I; // Index register
	unsigned short pc; // Program counter

	unsigned char delay_timer; // Delay timer
	unsigned char sound_timer; // Sound timer

	unsigned short stack[16]; // Stack
	unsigned short sp; // Stack pointer

};

Chip8::Chip8()
{

}

Chip8::~Chip8()
{
}

void Chip8::initialize()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	unsigned char chip8_fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// Clear display
	for (int i = 0; i < 64 * 32; i++)
	{
		gfx[i] = 0;
	}

	// Clear stack
	for (int i = 0; i < 16; i++)
	{
		stack[i] = 0;
	}

	// Clear registers V0-VF
	for (int i = 0; i < 16; i++)
	{
		V[i] = 0;
	}

	// Clear memory
	for (int i = 0; i < 4096; i++)
	{
		memory[i] = 0;
	}

	// Load fontset into memory
	for (int i = 0; i < 80; i++)
	{
		memory[i] = chip8_fontset[i];
	}

	// Reset Timers
	delay_timer = 0;
	sound_timer = 0;

	// Clear screen once
	drawFlag = true;
};

void Chip8::emulateCycle() 
{
	// Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decode Opcode
	switch (opcode & 0xF000) // Extract first nibble
	{

	case 0x0000: // First nibble is 0
		switch (opcode & 0x000F)
		{
			case 0x0000: // Clears the screen
				for (int i = 0; i < 2048; i++)
				{
					gfx[i] = 0x0;
				}
				pc += 2;
			break;

			case 0x000E: //Returns from a subroutine.
				sp--; // Decrement stack pointer to get to previous address 
				pc = stack[sp]; // set the program counter to the value in the stack where we came from
				pc += 2; // Go to next instruction
			break;

			default:
				printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
		}
	break;

	case 0x1000: // 1NNN Jumps to address NNN.
		pc = opcode & 0x0FFF;
	break;

	case 0x2000: // 2NNN Calls subroutine at NNN
		stack[sp] = pc;
		sp++;
		pc = opcode & 0x0FFF;
	break;

	case 0x3000: // 3XNN Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
	break;

	case 0x4000: // 4XNN Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
	break;

	case 0x5000: // 5XY0 Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
		if (V[(opcode & 0x0F00) >> 8] == V[opcode & 0x00F0] >> 4)
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
	break;

	case 0x6000: // 6XNN Sets VX to NN.
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
	break;
	
	case 0x7000: // 7XNN Adds NN to VX.
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
	break;

	case 0x8000:
		switch (opcode & 0x000F)
		{

		case 0x0000: // Sets VX to the value of VY.
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
		break;

		case 0x0001: // 8XY1 Sets VX to VX or VY. (Bitwise OR operation)
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		break;

		case 0x0002: // 8XY2 Sets VX to VX and VY. (Bitwise AND operation)
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		break;

		case 0x0003: // 8XY3 Sets VX to VX xor VY. (Bitwise XOR operation)
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		break;

		case 0x0004: // 8XY4 Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
			if ((V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]) < 256)
			{
				V[0xF] = 0;
			}
			else
			{
				V[0xF] = 1;
			}

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
		break;

		case 0x0005: // 8XY5 VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			if ((V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4]) >= 0)
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
		break;

		case 0x0006: // 8XY6 Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
		break;

		case 0x0007: // 8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			if ((V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8]) >= 0)
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
		break;

		case 0x000E: // 8XYE Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
			V[0xF] = V[(opcode & 0x0F000) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
		break;

		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
		}
	break;

	case 0x9000: // 9XY0 Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
		if (V[(opcode & 0x0F000) >> 8] != V[(opcode & 0x00F0) >> 4])
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
	break;

	case 0xA000: // ANNN Sets I to the address NNN.
		I = opcode & 0x0FFF;
		pc += 2;
	break;
	
	case 0xB000: // BNNN Jumps to the address NNN plus V0.
		pc = (opcode & 0x0FFF) + V[0x0];
	break;

	case 0xC000: // CXNN Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
	break;

	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
				 // Each row of 8 pixels is read as bit-coded starting from memory location I; 
				 // I value doesn't change after the execution of this instruction. 
				 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
				 // and to 0 if that doesn't happen
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E: //EX9E: Skips the next instruction if the key stored in VX is pressed
			if (V[(opcode & 0x0F00) >> 8] != 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
		break;

		case 0x00A1: // EXA1 Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
		break;

		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
		}
	break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
			case 0x0007: // FX07 Sets VX to the value of the delay timer.
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				pc += 2;
			break;			case 0x000A: // FX0A Key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
			{
				bool keypress = false;

				for (int i = 0; i < 16; i++)
				{
					if (key[i] != 0)
					{
						V[(opcode & 0x0F00) >> 8] = i;
						keypress = true;
					}
				}

				if (!keypress)
				{
					return;
				}

				pc += 2;
			}
			break;

			case 0x0015: // FX15 Sets the delay timer to VX.
				delay_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;

			case 0x0018: // FX18 Sets the sound timer to VX.
				sound_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
			break;

			case 0x001E: //FX1E Adds VX to I. VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't. This is undocumented feature of the CHIP-8 and used by Spacefight 2091! game.
				if ((I + V[(opcode & 0x0F00) >> 8]) > 0xFFF)
				{
					V[0xF] = 1;
				}
				else
				{
					V[0xF] = 0;
				}
				I += V[(opcode & 0x0F00) >> 8];
				pc += 2;
			break;

			case 0x0029: // FX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				I = V[(opcode & 0x0F00) >> 8] * 0x5;
				pc += 2;
			break;

			case 0x0033: // FX33 Tottally copy pasted this IDK WTF it does
				memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
				memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
				pc += 2;
			break;

			case 0x0055: // FX55 Stores V0 to VX (including VX) in memory starting at address I.[4]
				for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
				{
					memory[I + i] = V[i];
				}

				// On the original interpreter, when the operation is done, I = I + X + 1.
				I += ((opcode & 0x0F00) >> 8) + 1;
				pc += 2;
			break;

			case 0x0065: // FX65 Fills V0 to VX (including VX) with values from memory starting at address I.[4]
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
				{
					V[i] = memory[I + i];
				}

				// On the original interpreter, when the operation is done, I = I + X + 1.
				I += ((opcode & 0x0F00) >> 8) + 1;
				pc += 2;
			break;

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
		}
	
	break;

	default:
		printf("Unknown opcode: 0x%X\n", opcode);
	}

	// Update timers
	if (delay_timer > 0)
	{
		delay_timer--;
	}

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
		{
			printf("Beep!");
		}
		sound_timer--;
	}
}

bool Chip8::loadFile(const char * filename) {
	initialize();
	printf("Loading: %s\n", filename);

	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return false;
	}

	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error", stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if ((4096 - 512) > lSize)
	{
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}

void Chip8::debugRender()
{
	// Draw
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (gfx[(y * 64) + x] == 0)
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}