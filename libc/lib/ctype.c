
int isalnum( int ch ) {
	if ('0' <= ch && ch <= '9') return 1;
	if ('A' <= ch && ch <= 'Z') return 1;
	if ('a' <= ch && ch <= 'z') return 1;
	return 0;
}

int isalpha( int ch ) {
	if ('A' <= ch && ch <= 'Z') return 1;
	if ('a' <= ch && ch <= 'z') return 1;
	return 0;
}

int isdigit( int ch ) {
	return ('0' <= ch && ch <= '9');
}

int isspace( int ch ) {
	switch (ch) {
		case 0x20:
		case 0x0c:
		case 0x0a:
		case 0x0d:
		case 0x09:
		case 0x0b:
			return 1;
		default:
			return 0;
	}
}

int islower( int ch ) {
	return ('a' <= ch && ch <= 'z');
}

int isupper( int ch ) {
	return ('A' <= ch && ch <= 'Z');
}

int tolower( int ch ) {
	if (!isupper(ch)) return ch;
	return ch + 32;
}

int toupper( int ch ) {
	if (!islower(ch)) return ch;
	return ch - 32;
}
