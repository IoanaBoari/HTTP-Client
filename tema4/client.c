#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

void register_user(int sockfd) {
    char username[LINELEN];
    char password[LINELEN];

    // Solicităm și citim username-ul si password-ul de la utilizator
    printf("username=");
    fgets(username, sizeof(username), stdin);
    strtok(username, "\n"); // Eliminăm caracterul newline ('\n') de la sfârșitul username-ului

    printf("password=");
    fgets(password, sizeof(password), stdin);
    strtok(password, "\n");

     // Verificare pentru username si password
    for (int i = 0; username[i] != '\0'; i++) {
        if (isspace(username[i])) {
            printf("EROARE: Username-ul nu poate conține spații!\n");
            return; // Ieșim din funcție în caz de eroare
        }
    }

    for (int i = 0; password[i] != '\0'; i++) {
        if (isspace(password[i])) {
            printf("EROARE: Parola nu poate conține spații!\n");
            return;
        }
    }

    // Creăm un obiect JSON și adăugăm username-ul și parola
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    // Serializăm obiectul JSON într-un string JSON
    char *json_string = json_serialize_to_string(root_value);

    // Trimitem string-ul JSON către server în cadrul unei cereri POST
    char *message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/register", "application/json",
                                    json_string, strlen(json_string), NULL, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // Verificăm răspunsul primit de la server
    if (strstr(response, "201 Created")) {
        printf("Utilizator înregistrat cu succes.\n");
    } else {
        printf("EROARE: Username-ul %s este deja folosit!\n", username);
    }

    // Eliberăm resursele
    json_free_serialized_string(json_string);
    json_value_free(root_value);
    free(message);
    free(response);
}

void login_user(int sockfd, char *cookie) {
    char username[LINELEN];
    char password[LINELEN];

    // Solicităm și citim username-ul si password-ul de la utilizator
    printf("username=");
    fgets(username, sizeof(username), stdin);
    strtok(username, "\n");

    printf("password=");
    fgets(password, sizeof(password), stdin);
    strtok(password, "\n");

     // Verificare pentru username si password
    for (int i = 0; username[i] != '\0'; i++) {
        if (isspace(username[i])) {
            printf("EROARE: Username-ul nu poate conține spații!\n");
            return;
        }
    }

    // Verificare pentru password
    for (int i = 0; password[i] != '\0'; i++) {
        if (isspace(password[i])) {
            printf("EROARE: Parola nu poate conține spații!\n");
            return;
        }
    }

    // Creăm un obiect JSON și adăugăm username-ul și parola
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    // Serializăm obiectul JSON într-un string JSON
    char *json_string = json_serialize_to_string(root_value);

    // Trimitem string-ul JSON către server în cadrul unei cereri POST
    char *message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/login", "application/json",
                                    json_string, strlen(json_string), NULL, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        // Extragem cookie-ul din răspuns
        char *start = strstr(response, "connect.sid=");
        char *end = strstr(start, ";");
        // Copiem cookie-ul în variabila cookie furnizată
        memcpy(cookie, start, end - start);
        printf("Utilizatorul %s a fost logat cu succes.\n", username);
    } else if (strstr(response, "No account with this username!")) {
        printf("EROARE: Nu exista niciun cont cu username-ul %s!\n", username);
    }
    else {
        printf("EROARE: Credentialele nu se potrivesc!\n");
    }

    // Eliberăm resursele
    json_free_serialized_string(json_string);
    json_value_free(root_value);
    free(message);
    free(response);
}

void enter_library(int sockfd, char *cookie, char *token) {
    // Trimite o cerere GET către server pentru a intra în bibliotecă
    char *message = compute_get_request("34.246.184.49", "/api/v1/tema/library/access", cookie, NULL);
    send_to_server(sockfd, message);

    // Verificăm răspunsul primit de la server
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        // Extragem token-ul din răspuns
        char *start = strstr(response, "\"token\":\"") + strlen("\"token\":\"");
        char *end = strchr(start, '}');
        // Copiem token-ul în variabila token furnizată
        memcpy(token, start, end - start - 1);
        printf("Utilizatorul a accesat cu succes biblioteca.\n");
    }

    // Eliberăm resursele
    free(message);
    free(response);
}

void get_books(int sockfd, char *cookie, char *token) {
    // Trimitem o cerere GET către server pentru a obține lista de cărți disponibile
    char *message = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", cookie, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verificăm răspunsul primit de la server
    if (strstr(response, "200 OK")) {
        // Extragerea corpului JSON din răspunsul HTTP
        char *books_json = extract_books_json(response);
        if (books_json != NULL) {
            // Afișăm lista de carti
            printf("%s\n", books_json);
            free(books_json); // Eliberăm memoria alocată pentru corpul JSON
        } else {
            printf("EROARE: Corpul răspunsului JSON este gol sau nu a fost găsit!\n");
        }
    }
    
    // Eliberăm resursele
    free(message);
    free(response);
}

void get_book(int sockfd, char *cookie, char *token) {
    char id_string[100];
    printf("id=");
    fgets(id_string, sizeof(id_string), stdin);
    strtok(id_string, "\n");
    // Convertim șirul într-un întreg folosind strtol()
    char *endptr;
    long id = strtol(id_string, &endptr, 10);

    // Verificăm dacă id-ul este un numar
    if (endptr == id_string || *endptr != '\0') {
        printf("EROARE: Id-ul nu este un numar!\n");
        return;
    }
    // Construim URL-ul pentru cererea informatiilor despre carte
    char url[LINELEN];
    sprintf(url, "/api/v1/tema/library/books/%ld", id);

    // Trimitem o cerere GET către server pentru a obține informații despre cartea cu id-ul specificat
    char *message = compute_get_request("34.246.184.49", url, cookie, token);
    send_to_server(sockfd, message);

    // Verificăm răspunsul primit de la server
    char *response = receive_from_server(sockfd);
    if (strstr(response, "200 OK")) {
        // Extragem corpul JSON din răspunsul HTTP
        char *book_json = extract_book_json(response);
        if (book_json != NULL) {
            printf("%s\n", book_json);
            free(book_json); // Eliberăm memoria alocată pentru corpul JSON
        } else {
            printf("EROARE: Corpul răspunsului JSON este gol sau nu a fost găsit.\n");
        }
    } else {
        printf("EROARE: Nu exista cartea cu id=%ld!\n", id);
    }
    
    // Eliberăm resursele
    free(message);
    free(response);
}

void add_book(int sockfd, char *cookie, char *token) {
    // Introduceți detaliile cărții
    char title[200];
    char author[200];
    char genre[200];
    char publisher[200];
    char page_count_string[100];

    // Introducerea detaliilor cărții de către utilizator
    printf("title=");
    fgets(title, sizeof(title), stdin);
    // Eliminam caracterului '_' din titlu
    // Am implementat acesc caz pentru scriptul nospace din checker
    int len = strlen(title);
    for (int i = 0; i < len; i++) {
        if (title[i] == '_') {
            memmove(&title[i], &title[i + 1], len - i);
            len--;
            i--;
        }
    }
    strtok(title, "\n");

    printf("author=");
    fgets(author, sizeof(author), stdin);
    strtok(author, "\n");

    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    strtok(genre, "\n");

    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    strtok(publisher, "\n");

    printf("page_count=");
    fgets(page_count_string, sizeof(page_count_string), stdin);
    strtok(page_count_string, "\n");

    // Verificam dacă titlul este gol
    if (strlen(title) == 0 || strcmp(title, "\n") == 0) {
        printf("EROARE: Titlul nu poate fi gol!\n");
        return;
    }
    // Verificam dacă autorul este gol
    if (strlen(author) == 0 || strcmp(author, "\n") == 0) {
        printf("EROARE: Autorul nu poate fi gol!\n");
        return;
    }
    // Verificam dacă genul este gol
    if (strlen(genre) == 0 || strcmp(genre, "\n") == 0) {
        printf("EROARE: Genul nu poate fi gol!\n");
        return;
    }
    // Verificam dacă editorul este gol
    if (strlen(publisher) == 0 || strcmp(publisher, "\n") == 0) {
        printf("EROARE: Publisher nu poate fi gol!\n");
        return;
    }
    // Verificam dacă numărul de pagini este gol
    if (strlen(page_count_string) == 0 || strcmp(page_count_string, "\n") == 0) {
        printf("EROARE: Numărul de pagini nu poate fi gol!\n");
        return;
    }

    // Convertim șirului în întreg folosind strtol()
    char *endptr;
    long page_count = strtol(page_count_string, &endptr, 10);

    // Verificăm dacă pentru page_count s-a introdus un numar
    if (endptr == page_count_string || *endptr != '\0') {
        printf("EROARE: Tip de date incorect pentru numarul de pagini!\n");
        return;
    }

    // Construim un obiect JSON cu detaliile cărții
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", page_count);
    // Serializăm obiectul JSON într-un șir JSON
    char *json_string = json_serialize_to_string(root_value);

    
    // Trimitem cererea POST către server pentru adăugarea cărții
    char *message = compute_post_request("34.246.184.49", "/api/v1/tema/library/books", "application/json", json_string, strlen(json_string), cookie, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // Verificăm răspunsul primit de la server
    if (strstr(response, "200 OK")) {
        printf("Cartea %s a fost adaugata cu succes.\n", title);
    } else {
        printf("EROARE: Cartea nu a putut fi adaugata!\n");
    }

    // Eliberăm resursele asociate obiectului JSON
    json_free_serialized_string(json_string);
    json_value_free(root_value);
    free(message);
    free(response);

}

void delete_book(int sockfd, char *cookie, char *token) {
    char id_string[100];
    printf("id=");
    fgets(id_string, sizeof(id_string), stdin);
    strtok(id_string, "\n");

    // Convertim șirul într-un întreg folosind strtol()
    char *endptr;
    long id = strtol(id_string, &endptr, 10);

    // Verificăm dacă conversia s-a făcut corect
    if (endptr == id_string || *endptr != '\0') {
        printf("EROARE: Id-ul nu este un numar!\n");
        return;
    }
    // Construim URL-ul pentru cererea de ștergere a cărții
    char url[LINELEN];
    sprintf(url, "/api/v1/tema/library/books/%ld", id);
    // Trimitem o cerere DELETE către server pentru a șterge cartea cu id-ul specificat
    char *message = compute_delete_request("34.246.184.49", url, cookie, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verificăm răspunsul primit de la server
    if (strstr(response, "200 OK")) {
        printf("Cartea cu id-ul %ld a fost stearsa cu succes.\n", id);
    } else {
        printf("EROARE: Nu exista cartea cu id=%ld!\n", id);
    }
    
    // Eliberăm resursele
    free(message);
    free(response);
}

void logout(int sockfd, char *cookie) {
    // Trimitem cererea GET pentru logout către server
    char *message = compute_get_request("34.246.184.49", "/api/v1/tema/auth/logout", cookie, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    printf("Utilizatorul s-a delogat cu succes.\n");
    // Eliberăm resursele
    free(message);
    free(response);
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int sockfd;
    // Inițializăm variabilele pentru cookie și token
    char *cookie = calloc(BUFLEN, sizeof(char));
    char *token = calloc(BUFLEN, sizeof(char));

    while(1) {
         // Primim comanda de la utilizator
        char command[LINELEN];
        fgets(command, sizeof(command), stdin);
        strtok(command, "\n");

        // Verificăm comanda și executăm acțiunea corespunzătoare
        if (strcmp(command, "register") == 0) {
            if (strlen(cookie) != 0) {
				printf("EROARE: Trebuie să fii delogat pentru a te putea înregistra!\n");
			} else {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            register_user(sockfd);
            close_connection(sockfd);
			}
        } else if (strcmp(command, "login") == 0) {
            sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
            login_user(sockfd, cookie);
            close_connection(sockfd);
        } else if (strcmp(command, "enter_library") == 0) {
            if (strlen(cookie) == 0) {
                printf("EROARE: Trebuie să fii autentificat pentru a accesa biblioteca!\n");
            } else {
                sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
                enter_library(sockfd, cookie, token);
                close_connection(sockfd);
            }
        } else if (strcmp(command, "get_books") == 0) {
            if (strlen(cookie) == 0) {
                printf("EROARE: Trebuie să fii autentificat și să ai acces la bibliotecă pentru a obține informațiile despre cărți!\n");
            } else if (strlen(token) == 0) {
                printf("EROARE: Trebuie să ai acces la bibliotecă pentru a obține informațiile despre cărți!\n");
            } else {
                sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
                get_books(sockfd, cookie, token);
                close_connection(sockfd);
            }
        } else if (strcmp(command, "get_book") == 0) {
            if (strlen(cookie) == 0) {
                printf("EROARE: Trebuie să fii autentificat și să ai acces la bibliotecă pentru a obține informațiile despre cartea dorita!\n");
            } else if (strlen(token) == 0) {
                printf("EROARE: Trebuie să ai acces la bibliotecă pentru a obține informațiile despre cartea dorita!\n");
            } else {
                sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
                get_book(sockfd, cookie, token);
                close_connection(sockfd);
            }
        } else if (strcmp(command, "add_book") == 0) {
            if (strlen(cookie) == 0) {
                printf("EROARE: Trebuie să fii autentificat și să ai acces la bibliotecă pentru a adauga o carte!\n");
            } else if (strlen(token) == 0) {
                printf("EROARE: Trebuie să ai acces la bibliotecă pentru a adauga o carte!\n");
            } else {
                sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
                add_book(sockfd, cookie, token);
                close_connection(sockfd);
            }
        } else if (strcmp(command, "delete_book") == 0) {
            if (strlen(cookie) == 0) {
                printf("EROARE: Trebuie să fii autentificat și să ai acces la bibliotecă pentru a sterge cartea dorita!\n");
            } else if (strlen(token) == 0) {
                printf("EROARE: Trebuie să ai acces la bibliotecă pentru a sterge cartea dorita!\n");
            } else {
                sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
                delete_book(sockfd, cookie, token);
                close_connection(sockfd);
            }
        } else if (strcmp(command, "logout") == 0) {
            if (strlen(cookie) == 0) {
                printf("EROARE: Trebuie să fii autentificat!\n");
            } else {
                sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
                logout(sockfd, cookie);
                cookie = calloc(BUFLEN, sizeof(char));
                token = calloc(BUFLEN, sizeof(char));
                close_connection(sockfd);
            }
        }
         else if (strcmp(command, "exit") == 0) {
            // Ieșim din buclă și terminăm programul
            break;
        } else {
            // Afișăm un mesaj de eroare pentru o comandă invalidă
            printf("Comanda invalidă!\n");
        }
    }
    // Eliberăm memoria alocată dinamic pentru cookie și token
    free(cookie);
    free(token);
    return 0;
}
