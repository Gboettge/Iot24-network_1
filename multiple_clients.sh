echo "Kompilerar klient..."
g++ -std=c++11 client.cpp -o client -pthread || { echo "Kompilering misslyckades"; exit 1; }

# Starta 20 klientinstanser
echo "Startar 20 klienter..."

for i in {1..20}; do
    ./client &
    echo "Klient $i startad"
    sleep 1
done

# Vänta tills alla bakgrundsjobb är klara (om du vill)
wait
echo "Alla klienter har körts klart."

# chmod +x multiple_clients.sh
# ./multiple_clients.sh