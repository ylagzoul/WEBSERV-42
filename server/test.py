import time
def get_cgi_path_info(script_path, extension=".cgi"):
    """
    Extract PATH_INFO from CGI request
    Example: "/cgi-bin/script.cgi/user/profile" -> "/user/profile"
    """
    ext_pos = script_path.find(extension)
    if ext_pos != -1:
        return script_path[ext_pos + len(extension):]
    return ""

# Usage
cgi_uri = "/cgi-bin/test.cgi/user/john/details"
path_info = get_cgi_path_info(cgi_uri, ".cgi")
print(path_info)  # Output: /user/john/details



# while True:
#     pass


# time.sleep(20)