# {{ version.name }} {{ version.version }}
{% if version.filedates is not none -%}
The original timestamps of files found in the `tar` archive are as follows:

```
{{ version.filedates|rtrim }}
```
{% else -%}
*This version was not recovered from a tar archive, and file timestamps are
unavailable.*
{% endif %}
