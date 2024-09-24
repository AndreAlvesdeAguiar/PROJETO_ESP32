import requests
import mysql.connector
import os
import time

# URL da API local do ESP32
url = "http://192.168.15.20/dados"

# Função para gravar dados no banco de dados
def save_data():
    try:
        # Fazendo a requisição para a API
        response = requests.get(url)
        data = response.json()

        # Conectando ao banco de dados MySQL
        connection = mysql.connector.connect(
            host=os.getenv('MYSQL_HOST', 'mysql'),
            database=os.getenv('MYSQL_DATABASE', 'esp_data'),
            user=os.getenv('MYSQL_USER', 'user'),
            password=os.getenv('MYSQL_PASSWORD', '1234')  # Certifique-se de usar a senha correta
        )

        if connection:
            cursor = connection.cursor()

            # Inserindo os dados na tabela
            query = """INSERT INTO sensor_data (temp_bmp180, pressure_bmp180, temp_aht21, humidity_aht21, aqi_ens160, tvoc_ens160, eco2_ens160)
                       VALUES (%s, %s, %s, %s, %s, %s, %s)"""
            values = (data.get('temp_bmp180'), data.get('pressure_bmp180'),
                      data.get('temp_aht21'), data.get('humidity_aht21'),
                      data.get('aqi_ens160'), data.get('tvoc_ens160'),
                      data.get('eco2_ens160'))

            cursor.execute(query, values)
            connection.commit()
            print("Dados inseridos com sucesso")

    except mysql.connector.Error as e:
        print(f"Erro ao conectar ao MySQL: {e}")

    except requests.exceptions.RequestException as e:
        print(f"Erro ao fazer a requisição para a API: {e}")

    finally:
        # Fechando a conexão se ela foi aberta
        if 'connection' in locals():
            cursor.close()
            connection.close()
            print("Conexão ao MySQL foi encerrada")

# Loop para gravar dados a cada minuto
while True:
    save_data()
    time.sleep(5)  # Espera 60 segundos antes de executar novamente
