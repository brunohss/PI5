#!/usr/bin/env python3
"""
PrintSense Log Analyzer
Ferramenta para análise de logs do sistema PrintSense

Uso:
    python analyze_logs.py log.csv
    python analyze_logs.py log.csv --material PLA --plot
"""

import pandas as pd
import matplotlib.pyplot as plt
import argparse
import sys
from datetime import datetime

# Thresholds por material
THRESHOLDS = {
    'PLA': {'temp_min': 18, 'temp_max': 28, 'hum_min': 40, 'hum_max': 60},
    'PETG': {'temp_min': 20, 'temp_max': 30, 'hum_min': 30, 'hum_max': 50},
    'ABS': {'temp_min': 22, 'temp_max': 32, 'hum_min': 20, 'hum_max': 40},
    'RESINA': {'temp_min': 20, 'temp_max': 25, 'hum_min': 40, 'hum_max': 60},
}

def load_log(filepath):
    """Carrega arquivo de log CSV"""
    try:
        df = pd.read_csv(filepath)
        df['timestamp'] = pd.to_datetime(df['timestamp'])
        return df
    except Exception as e:
        print(f"Erro ao carregar arquivo: {e}")
        sys.exit(1)

def analyze_conditions(df, material='PLA'):
    """Analisa condições ambientais"""
    thresh = THRESHOLDS.get(material, THRESHOLDS['PLA'])
    
    print(f"\n{'='*60}")
    print(f"ANÁLISE DE CONDIÇÕES AMBIENTAIS - Material: {material}")
    print(f"{'='*60}\n")
    
    # Estatísticas gerais
    print("📊 ESTATÍSTICAS GERAIS")
    print(f"Período: {df['timestamp'].min()} até {df['timestamp'].max()}")
    print(f"Total de leituras: {len(df)}")
    print(f"Duração: {(df['timestamp'].max() - df['timestamp'].min())}\n")
    
    # Temperatura
    print("🌡️  TEMPERATURA")
    print(f"   Média: {df['temperature'].mean():.1f}°C")
    print(f"   Mínima: {df['temperature'].min():.1f}°C")
    print(f"   Máxima: {df['temperature'].max():.1f}°C")
    print(f"   Ideal: {thresh['temp_min']}-{thresh['temp_max']}°C")
    
    temp_ok = ((df['temperature'] >= thresh['temp_min']) & 
               (df['temperature'] <= thresh['temp_max'])).sum()
    print(f"   ✅ Dentro da faixa: {temp_ok/len(df)*100:.1f}% do tempo\n")
    
    # Umidade
    print("💧 UMIDADE")
    print(f"   Média: {df['humidity'].mean():.1f}%")
    print(f"   Mínima: {df['humidity'].min():.1f}%")
    print(f"   Máxima: {df['humidity'].max():.1f}%")
    print(f"   Ideal: {thresh['hum_min']}-{thresh['hum_max']}%")
    
    hum_ok = ((df['humidity'] >= thresh['hum_min']) & 
              (df['humidity'] <= thresh['hum_max'])).sum()
    print(f"   ✅ Dentro da faixa: {hum_ok/len(df)*100:.1f}% do tempo\n")
    
    # Luminosidade
    print("💡 LUMINOSIDADE")
    print(f"   Média: {df['light'].mean():.0f} lux")
    print(f"   Mínima: {df['light'].min():.0f} lux")
    print(f"   Máxima: {df['light'].max():.0f} lux\n")
    
    # Status
    print("📈 STATUS GERAL")
    status_counts = df['status'].value_counts()
    for status, count in status_counts.items():
        percent = count/len(df)*100
        emoji = '✅' if status == 'IDEAL' else '⚠️' if status == 'BOM' else '❌'
        print(f"   {emoji} {status}: {count} leituras ({percent:.1f}%)")
    
    # Períodos problemáticos
    print("\n🚨 PERÍODOS CRÍTICOS")
    critical = df[(df['temperature'] < thresh['temp_min'] - 2) | 
                  (df['temperature'] > thresh['temp_max'] + 2) |
                  (df['humidity'] < thresh['hum_min'] - 10) |
                  (df['humidity'] > thresh['hum_max'] + 10)]
    
    if len(critical) > 0:
        print(f"   Encontrados {len(critical)} períodos críticos:")
        for idx, row in critical.head(5).iterrows():
            print(f"   - {row['timestamp']}: T={row['temperature']:.1f}°C, H={row['humidity']:.1f}%")
        if len(critical) > 5:
            print(f"   ... e mais {len(critical)-5} ocorrências")
    else:
        print("   ✅ Nenhum período crítico detectado!")
    
    # Recomendações
    print("\n💡 RECOMENDAÇÕES")
    temp_avg = df['temperature'].mean()
    hum_avg = df['humidity'].mean()
    
    if temp_avg < thresh['temp_min']:
        print("   🌡️  Temperatura média ABAIXO do ideal")
        print("      → Considere aquecedor ou mover impressoras para local mais quente")
    elif temp_avg > thresh['temp_max']:
        print("   🌡️  Temperatura média ACIMA do ideal")
        print("      → Considere ar-condicionado ou ventilação")
    
    if hum_avg < thresh['hum_min']:
        print("   💧 Umidade média ABAIXO do ideal")
        print("      → Risco de eletricidade estática. Use umidificador")
    elif hum_avg > thresh['hum_max']:
        print("   💧 Umidade média ACIMA do ideal")
        print("      → Filamento pode absorver água. Use desumidificador ou dry box")
    
    if df['temperature'].std() > 3:
        print("   ⚠️  Alta variação de temperatura detectada")
        print("      → Isole o ambiente ou use controle térmico automático")
    
    print(f"\n{'='*60}\n")

def plot_data(df, material='PLA'):
    """Gera gráficos dos dados"""
    thresh = THRESHOLDS.get(material, THRESHOLDS['PLA'])
    
    fig, axes = plt.subplots(2, 2, figsize=(15, 10))
    fig.suptitle(f'PrintSense - Análise Ambiental ({material})', fontsize=16)
    
    # Temperatura
    ax1 = axes[0, 0]
    ax1.plot(df['timestamp'], df['temperature'], label='Temperatura', color='red')
    ax1.axhline(y=thresh['temp_min'], color='green', linestyle='--', alpha=0.5, label='Mín Ideal')
    ax1.axhline(y=thresh['temp_max'], color='green', linestyle='--', alpha=0.5, label='Máx Ideal')
    ax1.fill_between(df['timestamp'], thresh['temp_min'], thresh['temp_max'], 
                     alpha=0.2, color='green', label='Faixa Ideal')
    ax1.set_ylabel('Temperatura (°C)')
    ax1.set_title('Temperatura ao longo do tempo')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Umidade
    ax2 = axes[0, 1]
    ax2.plot(df['timestamp'], df['humidity'], label='Umidade', color='blue')
    ax2.axhline(y=thresh['hum_min'], color='green', linestyle='--', alpha=0.5, label='Mín Ideal')
    ax2.axhline(y=thresh['hum_max'], color='green', linestyle='--', alpha=0.5, label='Máx Ideal')
    ax2.fill_between(df['timestamp'], thresh['hum_min'], thresh['hum_max'], 
                     alpha=0.2, color='green', label='Faixa Ideal')
    ax2.set_ylabel('Umidade (%)')
    ax2.set_title('Umidade ao longo do tempo')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # Luminosidade
    ax3 = axes[1, 0]
    ax3.plot(df['timestamp'], df['light'], label='Luminosidade', color='orange')
    ax3.set_ylabel('Luminosidade (lux)')
    ax3.set_title('Luminosidade ao longo do tempo')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    
    # Status
    ax4 = axes[1, 1]
    status_counts = df['status'].value_counts()
    colors = {'IDEAL': 'green', 'BOM': 'orange', 'RUIM': 'red'}
    status_colors = [colors.get(s, 'gray') for s in status_counts.index]
    ax4.bar(status_counts.index, status_counts.values, color=status_colors)
    ax4.set_ylabel('Quantidade de Leituras')
    ax4.set_title('Distribuição de Status')
    ax4.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    
    # Salvar
    output_file = f"analysis_{material}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"\n📊 Gráfico salvo: {output_file}")
    
    plt.show()

def export_report(df, material='PLA', output='report.txt'):
    """Exporta relatório em texto"""
    with open(output, 'w', encoding='utf-8') as f:
        thresh = THRESHOLDS[material]
        
        f.write("="*60 + "\n")
        f.write(f"RELATÓRIO PrintSense - {material}\n")
        f.write(f"Gerado em: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write("="*60 + "\n\n")
        
        f.write(f"Período: {df['timestamp'].min()} até {df['timestamp'].max()}\n")
        f.write(f"Total de leituras: {len(df)}\n\n")
        
        f.write("TEMPERATURA\n")
        f.write(f"  Média: {df['temperature'].mean():.1f}°C\n")
        f.write(f"  Mín/Máx: {df['temperature'].min():.1f}°C / {df['temperature'].max():.1f}°C\n")
        f.write(f"  Ideal: {thresh['temp_min']}-{thresh['temp_max']}°C\n\n")
        
        f.write("UMIDADE\n")
        f.write(f"  Média: {df['humidity'].mean():.1f}%\n")
        f.write(f"  Mín/Máx: {df['humidity'].min():.1f}% / {df['humidity'].max():.1f}%\n")
        f.write(f"  Ideal: {thresh['hum_min']}-{thresh['hum_max']}%\n\n")
        
        f.write("STATUS\n")
        for status, count in df['status'].value_counts().items():
            f.write(f"  {status}: {count} ({count/len(df)*100:.1f}%)\n")
    
    print(f"\n📄 Relatório exportado: {output}")

def main():
    parser = argparse.ArgumentParser(description='PrintSense Log Analyzer')
    parser.add_argument('logfile', help='Arquivo CSV de log')
    parser.add_argument('--material', '-m', default='PLA', 
                       choices=['PLA', 'PETG', 'ABS', 'RESINA'],
                       help='Material para análise (padrão: PLA)')
    parser.add_argument('--plot', '-p', action='store_true',
                       help='Gerar gráficos')
    parser.add_argument('--export', '-e', help='Exportar relatório (arquivo.txt)')
    
    args = parser.parse_args()
    
    # Carregar dados
    df = load_log(args.logfile)
    
    # Análise
    analyze_conditions(df, args.material)
    
    # Gráficos
    if args.plot:
        try:
            plot_data(df, args.material)
        except Exception as e:
            print(f"⚠️  Erro ao gerar gráficos: {e}")
            print("   Certifique-se de ter matplotlib instalado: pip install matplotlib")
    
    # Exportar relatório
    if args.export:
        export_report(df, args.material, args.export)

if __name__ == '__main__':
    main()
