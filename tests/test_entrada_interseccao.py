"""Apos make: python tests/test_entrada_interseccao.py"""
from pathlib import Path
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[1]
exe = repo / ('exe.exe' if (repo / 'exe.exe').exists() else 'exe')

def executar(entrada):
    p = subprocess.run([str(exe)], input=entrada, cwd=repo, text=True,
                       capture_output=True, encoding='utf-8', errors='replace', timeout=10)
    assert p.returncode == 0, p.stderr
    return p.stdout

with tempfile.TemporaryDirectory() as pasta:
    arquivo = Path(pasta) / 'primeiro automato.txt'
    arquivo.write_text((repo / 'exemplo_interseccao_afn_m1.txt').read_text())
    for caminho in ['exemplo_interseccao_afn_m1.txt', '"exemplo_interseccao_afn_m1.txt"',
                    f'  "{arquivo}"  ', f"'{arquivo}'"]:
        s = executar(f'7\n2\n\nnao-existe.txt\n{caminho}\n2\nexemplo_interseccao_afn_m2.txt\n0\n0\n')
        assert 'Nenhum arquivo informado' in s and 'tente novamente' in s
        assert 'M1 carregado com sucesso' in s and 'M2 carregado com sucesso' in s
        assert 'INTERSECCAO: L(M1) & L(M2)' in s
        assert 'AF ORIGINAL M1 ---' not in s
        assert s.index('Vamos encontrar as palavras') < s.index('Arquivo de M1')
    arquivo.write_text(',,,\n')
    s = executar(f'7\n2\n{arquivo}\nexemplo_interseccao_afn_m1.txt\n2\nexemplo_interseccao_afn_m2.txt\n0\n0\n')
    assert 'Arquivo invalido' in s and 'M2 carregado com sucesso' in s

for entrada in ['7\n2\n0\n0\n', '7\n2\n', '7\n2\n\n', '7\nx\n0\n0\n']:
    executar(entrada)
print('OK: ENTER vazio, arquivo ausente/invalido, nova tentativa, aspas, espacos, cancelamento e EOF.')
