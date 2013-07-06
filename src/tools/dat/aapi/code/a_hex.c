/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
    int width)
{
	char s[lineWidth];   // string with printable values
	int i = 0, j = 0;

	fprintf(output, " 0x%8.8X ", (unsigned int)addrPtr);
	while (lineWidth--) {
		if (i++ < width) {
			unsigned char c = (unsigned char)*addrPtr;
			fprintf(output, " %2.2X", ((unsigned int)c) & 0xff);
			if ( isprint(c) ) {
				s[j++] = c;
			} else {
				s[j++] = '.';
			}
		} else {
			fprintf(output, "   ");
		}
		if ( (groupWidth) && !(i == width) && !(i % groupWidth) ) {
			fprintf(output, " ");
			s[j++] = ' ';
		}
		addrPtr++;
	}
	s[j] = '\0';
	fprintf(output, "  %s\n", s);
}




/* print hex dump from specified address in memory
 */
void
a_hexDump(
	FILE *output,
	void *addrPtr,
	int length,
	int lineWidth,
	int groupWidth)
{
	int width;
	while(0 < length) {
		width = lineWidth < length ? lineWidth : length;
		a_hexDumpLine(output, (char *)addrPtr, lineWidth, groupWidth, width);
		addrPtr += lineWidth;
		length -= lineWidth;
	}
}







//END a_hex.c
