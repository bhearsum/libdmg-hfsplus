#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <dmg/dmg.h>
#include <dmg/filevault.h>

char endianness;

void TestByteOrder()
{
	short int word = 0x0001;
	char *byte = (char *) &word;
	endianness = byte[0] ? IS_LITTLE_ENDIAN : IS_BIG_ENDIAN;
}

int buildInOut(const char* source, const char* dest, AbstractFile** in, AbstractFile** out) {
	*in = createAbstractFileFromFile(fopen(source, "rb"));
	if(!(*in)) {
		printf("cannot open source: %s\n", source);
		return FALSE;
	}

	*out = createAbstractFileFromFile(strcmp(dest, "-") == 0 ? stdout : fopen(dest, "wb"));
	if(!(*out)) {
		(*in)->close(*in);
		printf("cannot open destination: %s\n", dest);
		return FALSE;
	}

	return TRUE;
}

int main(int argc, char* argv[]) {
	int partNum;
	AbstractFile* in;
	AbstractFile* out;
	int hasKey;
	
	TestByteOrder();
	
	if(argc < 4) {
		printf("usage: %s [extract|build|build2048|res|iso|dmg|attribute] <in> <out> (-k <key>) (partition)\n", argv[0]);
		return 0;
	}

	if(!buildInOut(argv[2], argv[3], &in, &out)) {
		return FALSE;
	}

	hasKey = FALSE;
	if(argc > 5) {
		if(strcmp(argv[4], "-k") == 0) {
			in = createAbstractFileFromFileVault(in, argv[5]);
			hasKey = TRUE;
		}
	}
	

	if(strcmp(argv[1], "extract") == 0) {
		partNum = -1;
		
		if(hasKey) {
			if(argc > 6) {
				sscanf(argv[6], "%d", &partNum);
			}
		} else {
			if(argc > 4) {
				sscanf(argv[4], "%d", &partNum);
			}
		}
		extractDmg(in, out, partNum);
	} else if(strcmp(argv[1], "build") == 0) {
		// TODO: signal sentinel in some way?
		buildDmg(in, out, SECTOR_SIZE, argc > 4 ? argv[4] : NULL);
	} else if(strcmp(argv[1], "build2048") == 0) {
		buildDmg(in, out, 2048, NULL);
	} else if(strcmp(argv[1], "res") == 0) {
		outResources(in, out);
	} else if(strcmp(argv[1], "iso") == 0) {
		convertToISO(in, out);
	} else if(strcmp(argv[1], "dmg") == 0) {
		convertToDMG(in, out);
	} else if(strcmp(argv[1], "attribute") == 0) {
		if(argc < 6) {
			printf("Not enough arguments: attribute <in> <out> <sentinel> <string>");
		}
		updateAttribution(in, out, argv[4], argv[5], strlen(argv[5]));
	}

	return 0;
}
