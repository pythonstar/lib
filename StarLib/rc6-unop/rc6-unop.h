
#pragma once

namespace Star
{
	namespace Encrypt
	{
		namespace Rc6
		{
			/* rc6 (TM)
			* Unoptimized sample implementation of Ron Rivest's submission to the
			* AES bakeoff.
			*
			* Salvo Salasio, 19 June 1998
			*
			* Intellectual property notes:  The name of the algorithm (RC6) is
			* trademarked; any property rights to the algorithm or the trademark
			* should be discussed with discussed with the authors of the defining
			* paper "The RC6(TM) Block Cipher": Ronald L. Rivest (MIT),
			* M.J.B. Robshaw (RSA Labs), R. Sidney (RSA Labs), and Y.L. Yin (RSA Labs),
			* distributed 18 June 1998 and available from the lead author's web site.
			*
			* This sample implementation is placed in the public domain by the author,
			* Salvo Salasio.  The ROTL and ROTR definitions were cribbed from RSA Labs'
			* RC5 reference implementation.
			*/


			/* RC6 is parameterized for w-bit words, b bytes of key, and
			* r rounds.  The AES version of RC6 specifies b=16, 24, or 32;
			* w=32; and r=20.
			*/

			#define w 32	/* word size in bits */
			#define r 20	/* based on security estimates */

			#define P32 0xB7E15163	/* Magic constants for key setup */
			#define Q32 0x9E3779B9

			/* derived constants */
			#define bytes   (w / 8)				/* bytes per word */
			#define c       ((b + bytes - 1) / bytes)	/* key in words, rounded up */
			#define R24     (2 * r + 4)
			#define lgw     5                       	/* log2(w) -- wussed out */

			/* Rotations */
			#define ROTL(x,y) (((x)<<(y&(w-1))) | ((x)>>(w-(y&(w-1)))))
			#define ROTR(x,y) (((x)>>(y&(w-1))) | ((x)<<(w-(y&(w-1)))))

			static unsigned int S[R24 - 1];		/* Key schedule */

			//------------------------------------------------------------------------
			void rc6_key_setup(unsigned char *K, int b);
			void rc6_block_encrypt(unsigned int *pt, unsigned int *ct);
			void rc6_block_decrypt(unsigned int *ct, unsigned int *pt);
			//------------------------------------------------------------------------
			void Encode(unsigned char*pIn,unsigned char*pOut,int nSize);
			void Decode(unsigned char*pIn,unsigned char*pOut,int nSize);
			void SetKey(unsigned char *K, int b);
			//------------------------------------------------------------------------
		}

	}
}