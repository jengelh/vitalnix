<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxutil_phash&nbsp;-- password hashing in Vitalnix</p>

<h1>Syntax</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxutil/libvxutil.h&gt;<br />
<br />
<b>bool</b> vxutil_phash(<b>const char *</b><i>key</i>, <b>const char *</b><i>salt</i>, <b>unsigned int</b> <i>algorithm</i>, <b>char **</b><i>result</i>);</code><br />
<br />
Link with <i>-lvxutil</i>.</p>

<h1>Description</h1>

<p class="block"><code>vxutil_phash()</code> is the password hashing/encryption
function in Vitalnix.</p>

<p class="block"><code><i>key</i></code> is a password or any random string
that is to be hashed/encrypted.</p>

<p class="block"><code><i>salt</i></code> is a random string used to perturb
the algorithm in varying number of ways. Different algorithms require different
salt strings. See below for details. <code><i>salt</i></code> may be
<code>NULL</code>, in which case <code>vxutil_phash()</code> will generate a
suitable random salt internally and use it.</p>

<h1>The base64 set</h1>

<p class="block">The base64 character set is [<code>./0-9A-Za-z</code>].</p>

<p class="block">While non-base64 characters in the salt work in Glibc's and
Vitalnix's implementations, you should not rely on this behavior, especially
because database backends (such as Shadow, see <a
href="vxdrv_shadow.7.php">vxdrv_shadow</a>(7)) put restrictions on what
characters to use. Using non-base64 characters will yield undefined results and
possibly corrupts your database or the hash when the hash is inserted into the
DB.</p>

<h1>Algorithm overview</h1>

<h2>DES crypt</h2>

<p class="block">DES crypt is selected using the <code>VXPHASH_DES</code>
constant for the <code><i>algorithm</i></code> argument. DES is a weak 56-bit
block cipher, and only the first eight characters of the
<code><i>key</i></code> are used. The salt must be a two-character string from
the base64 character set. Vitalnix relies on libc/libcrypt to provide the
implementation of this algorithm, so it is not available, for example, under
Microsoft Windows. <b>Use of DES is discouraged.</b></p>

<h2>MD5 hash</h2>

<p class="block">MD5 hashing is selected using the <code>VXPHASH_MD5</code>
constant. Both the key and salt (full length, unlike DES) are used to build the
128 bit digest.</p>

<p class="block">The salt consits of a ID prefix, the salt and optionally a
dollar sign. The ID is the three-character string "<code>$1$</code>", and the
salt is string of base64 characters with minimum length 0 and maximum length 8.
It will automatically be truncated if it is longer than this. Valid salts would
be, for example, "<code>$1$</code>", "<code>$1$$</code>", "<code>$1$ABC</code>"
and "<code>$1$longerthaneight$</code>".</p>

<h2>Blowfish crypt</h2>

<p class="block">Blowfish crypt is selected using
<code>VXPHASH_BLOWFISH</code>. It features a highly complex key schedule and
its number of rounds is tunable, which makes it fit for the long-term use.</p>

<p class="block">The salt consists of the ID prefix "<code>$2a$</code>", a salt
string of exactly 22 base64 characters and an optional dollar sign.</p>

<h2>NT4 hash</h2>

<p class="block">Windows NT hashing is selected using
<code>VXPHASH_SMBNT</code>. The <code><i>salt</i></code> argument is ignored.
The NT4 hash is a simple 128-bit MD4 digest of the <code><i>key</i></code> with
a few static transformations without any salt perturbation, and hence is
susceptible to dictionary and rainbow attacks. I discourage its use, though
unfortunately, Windows clients rely on this.</p>

<h2>SHA-256 hash</h2>

<p class="block">The SHA family of password hashing has been introduced just
recently (September 2007) as a successor to the DES crypt and MD5 hash methods.
It works similarly to the MD5 variant, but like Blowfish, the number of rounds
can also be tuned.</p>

<p class="block">The SHA-256 hash method can be selected using
<code>VXPHASH_SHA256</code>. The salt string is made up of the ID
"<code>$5$</code>", an optional rounds parameter
"<code>rounds=<i>N</i>$</code>", up to 16 base64 characters and an optional
dollar sign. The number of rounds, if provided, must be between 1,000 and
999,999,999 (inclusive). If it exceeds these limits, the value will be clamped.
A valid example: <code>$5$rounds=5000$abcdefghijklmnop$</code></p>

<h2>SHA-512 hash</h2>

<p class="block">The SHA-512 hash method can be selected using
<code>VXPHASH_SHA512</code>. Its ID is "<code>$6$</code>" and also allows a
rounds parameter and uses 16 base64 characters.</p>

<h1>Return value</h1>

<p class="block"><code>vxutil_phash()</code> will return a boolean, indicating
either success or failure. On success, <code><i>*result</i></code> is filled
with a pointer to an allocated region of memory containing the hash, which you
are supposed to free when you are done with it.</p>

<p class="block">Failure may arise if the wanted algorithm is not available or
a memory allocation failure occurred. The contents of the
<code><i>errno</i></code> variable are undefined.</p>

<h1>Example</h1>

<p class="code"><code>
char *result = NULL;<br />
if (vxutil_phash("password", "$2a$05$ABCDEFGHIJKLMNOPQRSTUV$", VXPHASH_BLOWFISH, &amp;result)) {<br />
&nbsp; &nbsp; printf("%s\n", result);<br />
&nbsp; &nbsp; free(result);<br />
}</code></p>

<?php include_once("Base-footer.php"); ?>
