<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>libvxutil&nbsp;-- helper functions for various tasks</p>

<h1>Syntax</h1>

<p class="code"><code><b>#</b>include
&lt;vitalnix/libvxutil/libvxutil.h&gt;</code></p>

<h1>Functions with separate manpages</h1>

<table border="1">
	<tr>
		<td><a href="vxutil_genpw.3.php">vxutil_genpw</a>(3)</td>
		<td>(Plaintext) password generation</td>
	</tr>
	<tr>
		<td><a href="vxutil_phash.3.php">vxutil_phash</a>(3)</td>
		<td>Password hashing/encryption</td>
	</tr>
</table>

<h1>Function: vxutil_azstr()</h1>

<p class="code"><code><b>char *</b>vxutil_azstr(<b>const char
*</b><i>string</i>);</code></p>

<p class="block"><code>vxutil_azstr()</code> will return a pointer to a
zero-length string if <i>string</i> is NULL, otherwise <i>string</i>. This
essentially makes sure the returned pointer can always be dereferenced. Actual use for this function is somewhere between laziness and cleaner code, e.g.:</p>

<p class="code"><code>strcmp(vxutil_azstr(foo), "bar") == 0</code></p>

<h1>Function: vxutil_have_display()</h1>

<p class="code"><code><b>bool</b> vxutil_have_display(<b>void</b>);</code></p>

<p class="block">This function can be used to determine if a graphical X
Windows environment is present by checking the <code>DISPLAY</code> environment
variable.</p>

<h1>Function: vxutil_now_iday()</h1>

<p class="code"><code><b>unsigned int</b>
vxutil_now_iday(<b>void</b>);</code></p>

<p class="block">Returns the current date measured in days from January 01
1970. This format is commonly used for password ageing monitoring.</p>

<h1>Function: vxutil_only_digits()</h1>

<p class="code"><code><b>bool</b> vxutil_only_digits(<b>const char
*</b><i>string</i>);</code></p>

<p class="block">Returns non-zero if there are only digits (as determined by
<code>isdigit()</code>) in the <code>string</code>.</p>

<h1>Function: vxutil_propose_home()</h1>

<p class="code"><code><b>char *</b>vxutil_propose_home(<b>char
*</b><i>dest</i>, <b>size_t</b> <i>size</i>, <b>const char *</b><i>base</i>,
<b>const char *</b><i>username</i>, <b>unsigned int</b>
<i>level</i>);</code></p>

<p class="block">Generates a home directory path, whose exact structure is
dependent on <code>level</code>. A larger level yields a deeper filesystem tree
depth, but reduces the number of concurrent inodes within a directory (this
reduces directory search time on legacy filesystems). Level 2 for example is
used by SourceForge's public servers.</p>

<table border="1">
	<tr>
		<td>Level 0</td>
		<td><code>/home/<i>username</i></code></td>
	</tr>
	<tr>
		<td>Level 1</td>
		<td><code>/home/<i>u</i>/<i>username</i></code></td>
	</tr>
	<tr>
		<td>Level 2</td>
		<td><code>/home/<i>u</i>/<i>us</i>/<i>username</i></code></td>
	</tr>
</table>

<p class="block">The result is put into <code><i>buf</i></code>. At most
<code><i>size</i>-1</code> characters are written and the result is always
<code>'\0'</code>-terminated.</p>

<h1>Function: vxutil_propose_lname()</h1>

<p class="code"><code><b>char *</b>vxutil_propose_lname(<b>char
*</b><i>dest</i>, <b>size_t</b> <i>size</i>, <b>const char *</b><i>surname</i>,
<b>const char *</b><i>firstname</i>);</code></p>

<p class="block">Creates a login name in the style of <code>FSSSSSS</code> out
of a real-world name. <code><i>surname</i></code> can be <code>NULL</code>,
while <code><i>firstname</i></code> must not be <code>NULL</code>. The two
strings must be UTF-8 encoded.</p>

<p class="block">There is special handling if the surname has multiple
parts&nbsp;-- let's take "van der Wies" as a fictional example. In this case,
we consider the last part as what we would like to have in the username.</p>

<h1>Function: vxutil_quote()</h1>

<p class="code"><code><b>char *</b>vxutil_quote(<b>const char *</b><i>src</i>,
<b>unsigned int</b> <i>type</i>, <b>char **</b><i>free_me</i>);</code></p>

<p class="block">Returns a the string <code><i>src</i></code> in its escaped
form according to <code><i>type</i></code>. The function returns
<code>NULL</code> on error, or a pointer to the escaped string on success. If
extra space was allocated, the pointer is stored in
<code><i>*free_me</i></code>, which is to be freed after usage. If no quoting
was done, because it was not needed, <code><i>*free_me</i></code> is set to
<code>NULL</code>. This allows you to use <code>free()</code>
unconditionally:</p>

<p class="code"><code>
char *free_me, *p;<br />
p = vxutil_quote("o'really?", VXQUOTE_SINGLE, &amp;free_me);<br />
printf("%s\n", p);<br />
free(free_me);</code></p>

<h1>Function: vxutil_replace_run()</h1>

<p class="code"><code><b>int</b> vxutil_replace_run(<b>const char
*</b><i>command</i>, <b>const struct</b> HXbtree
<b>*</b><i>varmap</i>);</code></p>

<p class="block">Replaces every occurrence of a <code>%{}</code>-tag in the
<code><i>command</i></code> string by the defined value from
<code><i>varmap</i></code>, then runs the expanded command using
<code>system()</code> and returns its status.</p>

<h1>Function: vxutil_slurp_file()</h1>

<p class="code"><code><b>char *</b>vxutil_slurp_file(<b>const char
*</b><i>filename</i>);</code></p>

<p class="block">Reads in the file specified by <code><i>filename</i></code> in
whole and returns a pointer to the newly allocated memory area, which should be
freed after usage. On error, <code>NULL</code> is returned and
<code><i>errno</i></code> will be set accordingly with the error from libc.</p>

<h1>Function: vxutil_string_iday()</h1>

<p class="code"><code><b>int</b> vxutil_string_iday(<b>const char
*</b><i>date</i>);</code></p>

<p class="block">Transforms the string pointed to by <code><i>date</i></code>,
which is of either format of <code>DD.MM.YYYY</code>, <code>MM/DD/YYYY</code>
or <code>YYYY-MM-DD</code> into an integer representing the days since January
01 1970. The function will return negative non-zero on (parsing) error.</p>

<h1>Function: vxutil_string_xday()</h1>

<p class="code"><code><b>int</b> vxutil_string_xday(<b>const char
*</b><i>date</i>);</code></p>

<p class="block">Transforms the string pointed to by <code><i>date</i></code>,
which is of either format of <code>DD.MM.YYYY</code>, <code>MM/DD/YYYY</code>
or <code>YYYY-MM-DD</code> into a BCD-style encoded integer representing the
date.</p>

<h1>Function: vxutil_valid_username()</h1>

<p class="code"><code><b>bool</b> vxutil_valid_username(<b>const char
*</b><i>username</i>);</code></p>

<p class="block">Returns <code>true</code> if the username does not contain any
illegal characters. Samba UNIX machine account names are also handled
(and will be accepted if they are valid).</p>

<h1>Function: vxuuid_vx3()</h1>

<p class="code"><code><b>char *</b>vxuuid_vx3(<b>const char
*</b><i>full_name</i>, <b>unsigned int</b> <i>xday</i>);</code></p>

<p class="block">Generate a UUID from the string <code><i>full_name</i></code>
and the integer <code><i>xday</i></code>. This is used for Data Sources which
come without an UUID, so one is generated based on the
(<code><i>name</i></code>, <code><i>date</i></code>) tuple we deem to be unique
within the Data Source. Returns a pointer to the newly allocated string
containing the UUID, or <code>NULL</code> on error.</p>

<?php include_once("Base-footer.php"); ?>
