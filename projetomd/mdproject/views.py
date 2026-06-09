from django.shortcuts import render
from django.http import JsonResponse, HttpResponse
import ctypes
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import zipfile


BASE_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = BASE_DIR.parent
REPO_ROOT = PROJECT_ROOT.parent
DECRYPT_OUTPUT_FILE = PROJECT_ROOT / 'textDencript.txt'

MENU_SOURCE_FILE = BASE_DIR / 'c_code' / 'menu.c'
MENU_LIBRARY_NAME = 'menu.dll' if os.name == 'nt' else 'menu.so'
MENU_LIBRARY_FILE = BASE_DIR / 'c_code' / MENU_LIBRARY_NAME


def normalize_menu_source(source_text):
    if not source_text:
        return source_text

    first_line_end = source_text.find('\n')
    if first_line_end == -1:
        first_line = source_text
    else:
        first_line = source_text[:first_line_end]

    if 'menu.c' in first_line and '#include' in first_line:
        prefix_index = first_line.find('menu.c')
        remainder = first_line[prefix_index + len('menu.c'):]
        if remainder.startswith('#include'):
            tail = source_text[first_line_end:] if first_line_end != -1 else ''
            return remainder + tail

    if first_line.strip().endswith('menu.c'):
        remainder = source_text[first_line_end + 1:] if first_line_end != -1 else ''
        if remainder.lstrip().startswith('#include'):
            return remainder

    return source_text


def load_menu_library():
    library_name = 'menu.dll' if os.name == 'nt' else 'menu.so'
    library_path = BASE_DIR / 'c_code' / library_name
    lib = ctypes.CDLL(str(library_path))
    # Parametros como strings: GMP aceita numeros arbitrariamente grandes,
    # mas ctypes nao conhece mpz_t. A interface usa const char* e o C
    # converte internamente com mpz_set_str(..., 10).
    lib.gerarChavePub.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    lib.gerarChavePub.restype = ctypes.c_int
    lib.encriptarMenu.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    lib.encriptarMenu.restype = ctypes.c_int
    lib.desencriptarMenu.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    lib.desencriptarMenu.restype = ctypes.c_int
    return lib


def read_decrypted_text():
    raw_text = DECRYPT_OUTPUT_FILE.read_bytes()
    try:
        return raw_text.decode('utf-8').strip()
    except UnicodeDecodeError:
        return raw_text.decode('latin-1', errors='replace').strip()


def replace_c_sources_from_zip(zip_file):
    with zipfile.ZipFile(zip_file) as archive:
        names = archive.namelist()

        menu_entry = None
        for name in names:
            if name.endswith('/'):
                continue
            base_name = Path(name).name
            if base_name == 'menu.c':
                menu_entry = name

        if not menu_entry:
            return False, 'O zip precisa conter menu.c.'

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_dir_path = Path(temp_dir)

            target_path = temp_dir_path / 'menu.c'
            with archive.open(menu_entry) as source, open(target_path, 'wb') as destination:
                shutil.copyfileobj(source, destination)

            source_text = target_path.read_text(encoding='utf-8', errors='replace')
            normalized_text = normalize_menu_source(source_text)
            if normalized_text != source_text:
                target_path.write_text(normalized_text, encoding='utf-8')

            validate_result = subprocess.run(
                ['gcc', '-fsyntax-only', str(target_path)],
                capture_output=True,
                text=True,
            )
            if validate_result.returncode != 0:
                return False, f'O menu.c enviado possui erro de sintaxe:\n{validate_result.stderr.strip()}'

            shutil.copyfile(temp_dir_path / 'menu.c', MENU_SOURCE_FILE)

    return True, ''


def compile_c_sources():
    if MENU_LIBRARY_FILE.exists():
        MENU_LIBRARY_FILE.unlink()

    if os.name == 'nt':
        return False, 'Compilacao automatica no Windows nao esta configurada.'

    build_result = subprocess.run(
        ['gcc', '-shared', '-fPIC', str(MENU_SOURCE_FILE), '-o', str(MENU_LIBRARY_FILE), '-lm'],
        capture_output=True,
        text=True,
    )
    if build_result.returncode != 0:
        return False, f'Falha ao compilar menu.c:\n{build_result.stderr.strip()}'

    return True, ''

# Create your views here.
def index(request):
    return render(request, 'mdproject/index.html', {})


def reset(request):
    if request.method == 'POST':
        for f in [PROJECT_ROOT / 'textEncript.txt',
                  PROJECT_ROOT / 'textDencript.txt',
                  PROJECT_ROOT / 'chavePub.txt']:
            if f.exists():
                f.unlink()
        return JsonResponse({'ok': True})
    return JsonResponse({'ok': False}, status=405)


def upload_c_zip(request):
    if request.method != 'POST':
        return HttpResponse('Metodo nao permitido.', status=405)

    zip_file = request.FILES.get('zip_file')
    if not zip_file:
        return render(request, 'mdproject/index.html', {
            'upload_error': 'Nenhum arquivo enviado.'
        })

    if not zip_file.name.lower().endswith('.zip'):
        return render(request, 'mdproject/index.html', {
            'upload_error': 'Envie um arquivo .zip valido.'
        })

    ok, error_message = replace_c_sources_from_zip(zip_file)
    if not ok:
        return render(request, 'mdproject/index.html', {
            'upload_error': error_message,
        })

    ok, error_message = compile_c_sources()
    if not ok:
        return render(request, 'mdproject/index.html', {
            'upload_error': error_message,
        })

    return render(request, 'mdproject/index.html', {
        'upload_success': 'menu.c substituido e biblioteca recompilada com sucesso.'
    })

def result(request):
     # Carregue a biblioteca compartilhada
    random_lib = ctypes.CDLL('./mdproject/c_code/crypt.dll')

        # Chame a função C para gerar um número aleatório
    random_number = random_lib.generate_random()
    print(random_number)
    return render(request, 'mdproject/result.html', {})



def save(request):
    try:
        if request.method == 'POST':
            primo1 = request.POST.get('primo1', '')
            primo2 = request.POST.get('primo2', '')
            e = request.POST.get('e', '')

            if primo1 and primo2 and e:
                random_lib = load_menu_library()
                status_code = random_lib.gerarChavePub(
                    primo1.encode(), primo2.encode(), e.encode()
                )
                if status_code == 1:
                    status = 'Primo 1 inválido'
                elif status_code == 2:
                    status = 'Primo 2 inválido'
                elif status_code == 3:
                    status = 'Expoente inválido'
                else:
                    status = 'Chaves públicas geradas com sucesso'

                return render(request, 'mdproject/save.html', {'status': status})
            else:
                return render(request, 'mdproject/save.html', {'status': 'Preencha todos os campos'})
        else:
            return render(request, 'mdproject/save.html', {'status': ''})
    except Exception as e:
        return JsonResponse({'error': str(e)})



def crypt(request):
    try:
        if request.method == 'POST':
            mensagem = request.POST.get('mensagem', '')
            n = request.POST.get('n', '')
            e = request.POST.get('e', '')
            if n and e:
                random_lib = load_menu_library()
                status = random_lib.encriptarMenu(
                    mensagem.encode(), n.encode(), e.encode()
                )
                if status == 0:
                    status = 'Mensagem criptografada com sucesso'
                else:
                    status = 'Erro na criptografia'
                return render(request, 'mdproject/crypt.html', {'status': status})
            else:
                return HttpResponse("Texto de entrada vazio.", status=400)
        else:
            return render(request, 'mdproject/crypt.html', {})
    except Exception as e:
        return JsonResponse({'error': str(e)})
    
    
    
def descrypt(request):
    try:
        if request.method == 'POST':
            p = request.POST.get('p', '')
            q = request.POST.get('q', '')
            e = request.POST.get('e', '')
            if p and q and e:
                random_lib = load_menu_library()
                status = random_lib.desencriptarMenu(
                    p.encode(), q.encode(), e.encode()
                )

                decrypted_text = ''
                if status == 0 and DECRYPT_OUTPUT_FILE.exists():
                    decrypted_text = read_decrypted_text()
                
                if status == 0:
                    status = 'Mensagem descriptografada com sucesso'
                
                return render(request, 'mdproject/decrypt.html', {
                    'status': status,
                    'decrypted_text': decrypted_text,
                })
            else:
                return HttpResponse("Texto de entrada vazio.", status=400)
        else:
            return render(request, 'mdproject/decrypt.html', {})
    except Exception as e:
        return JsonResponse({'error': str(e)})