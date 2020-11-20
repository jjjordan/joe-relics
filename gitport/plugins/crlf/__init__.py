def build_filter(args):
    return Filter(args)

class Filter():
    def __init__(self, args):
        pass

    def file_data_filter(self,file_data):
        file_ctx = file_data['file_ctx']
        file_name = file_data['filename']
        exts = ['.gif', '.ico', '.png']
        if not file_ctx.isbinary() and not any(file_name.endswith(ext) for ext in exts):
            file_data['data'] = file_data['data'].replace(b'\r\n', b'\n')
