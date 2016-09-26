TESTFILES="vcard1.vcf vcard2.vcf vcard3.vcf vcard4.vcf vcard6.vcf vcard7.vcf vcard8.vcf vcard9.vcf"

test -f FAILED && rm -f FAILED
for i in $TESTFILES;
  do perl ./checkvcard.pl ./tests/$i ;
done;

if [ -f FAILED ]; then
	echo ERROR
	exit 1
fi