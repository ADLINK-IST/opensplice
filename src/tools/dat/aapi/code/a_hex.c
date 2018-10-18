/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
