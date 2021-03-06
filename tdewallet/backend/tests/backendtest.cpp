#include <stdlib.h>
#include <stdio.h>

#include <tdeapplication.h>
#include <tqstring.h>

#include "tdewalletbackend.h"

int main(int argc, char **argv) {
   TDEApplication a(argc, argv, "tdewalletbackendtest");

   TDEWallet::Backend be("ktestwallet");
   printf("TDEWalletBackend constructed\n");

   TQByteArray apass, bpass, cpass;

   apass.duplicate("apassword", 9);
   bpass.duplicate("bpassword", 9);
   cpass.duplicate("cpassword", 9);

   printf("Passwords initialised.\n");
   int rc = be.close(apass);

   printf("be.close(apass) returned %d  (should be -255)\n", rc);

   rc = be.open(bpass);

   printf("be.open(bpass) returned %d  (should be 0 or 1)\n", rc);

   rc = be.close(bpass);

   printf("be.close(bpass) returned %d  (should be 0)\n", rc);

   rc = be.open(apass);

   printf("be.open(apass) returned %d  (should be negative)\n", rc);

   rc = be.open(bpass);

   printf("be.open(bpass) returned %d  (should be 0)\n", rc);

   return 0;
}


