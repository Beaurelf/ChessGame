# ♟️ ChessGame

Un jeu d'échecs moderne développé en C++ et Qt, avec génération de coups basée sur des bitboards et une interface graphique intuitive.

## 🎯 Fonctionnalités

### ✅ Implémenté

- **Architecture Bitboard** - Représentation efficace sur 64 bits pour une génération rapide des coups
- **Génération de Coups Légaux** - Règles complètes pour toutes les pièces (Cavalier, Fou, Tour, Dame, Roi)
- **Validation des Coups** - Filtrage automatique des coups qui mettraient le roi en échec
- **Captures de Pièces** - Mécaniques de capture complètes incluant la prise en passant
- **Promotion du Pion** - Dialogue interactif pour choisir la pièce de promotion
- **Surbrillance Visuelle** - Affichage des coups légaux et opportunités de capture
- **Interface Glisser-Déposer** - Déplacement fluide des pièces avec retour visuel
- **Horloges de Jeu** - Pendules d'échecs doubles pour le jeu compétitif
- **Cache des Coups** - Optimisation des performances pour les calculs répétés

### 🚧 En Développement

- Roque (petit et grand)
- Détection de pat
- Historique des coups et notation
- Manger en passant
- IA

## 🏗️ Architecture

### Composants Principaux

```
src/
├── model/
│   ├── ChessBitBoard      # Représentation bitboard (positions 64 bits)
├── controller/
│   └── ChessController    # Gestion de l'état du jeu et logique des tours
│   ├── MoveGenerator      # Calcul brut des coups (motifs géométriques)
│   └── MoveController     # Filtrage des coups légaux (validation avec le plateau)
└── view/
    ├── ChessBoard         # Scène Qt et rendu des pièces
    ├── PieceItem          # Pièces graphiques déplaçables
    └── CellItem           # Cases du plateau interactives
    └── Timer              # Gestion et affichage du timer
    └── Game               # Vue de l'application
    └── Home               # Vue d'accueil
```

### Patterns de Conception Clés

**Génération de Coups par Bitboard**

- Utilise des entiers 64 bits pour des requêtes ultra-rapides
- Masques séparés pour coups tranquilles vs captures
- Ray-casting pour pièces glissantes (Tour, Fou, Dame)

**Pattern Strategy**

- `MoveGenerator` délègue aux fonctions spécifiques par pièce
- `MoveController` filtre les coups illégaux selon l'état du plateau

**Architecture MVC**

- Modèle : Bitboard et logique des coups
- Vue : Éléments graphiques Qt
- Contrôleur : Règles du jeu et tours des joueurs

## 🔧 Stack Technique

- **Langage :** C++17
- **Système de Build :** qmake (.pro)
- **Représentation des Coups :** Bitboards 64 bits
- **Performance :** Recherche O(1), génération O(n)

## 🚀 Compilation & Exécution

### Prérequis

```bash
# Installation de Qt 6
sudo apt install qt6-base-dev qt6-tools-dev
```

### Compilation

```bash
# Avec Qt Creator (recommandé)
# 1. Ouvrir ChessGame.pro dans Qt Creator
# 2. Configurer le kit de compilation
# 3. Appuyer sur Ctrl+B pour compiler

# En ligne de commande
qmake ChessGame.pro
make
./ChessGame
```

## 🎮 Comment Jouer

1. **Démarrer une Partie** - Les pièces sont positionnées automatiquement
2. **Sélectionner une Pièce** - Cliquez sur n'importe quelle pièce du joueur actuel
3. **Voir les Coups Légaux** - Les cases surlignées montrent les coups valides (points pour coups tranquilles, surbrillance pour captures)
4. **Faire un Coup** - Glissez-déposez vers la case cible
5. **Promotion du Pion** - Choisissez la pièce d'amélioration en atteignant l'extrémité opposée

### Contrôles

- **Clic Gauche / Glisser** - Déplacer la pièce
- **Minuteur** - Bascule automatiquement entre les joueurs

## 📊 Performance de la Génération de Coups

| Type de Pièce | Algorithme                | Complexité   |
| ------------- | ------------------------- | ------------ |
| Pion          | Calcul direct             | O(1)         |
| Cavalier      | Calcul direct             | O(1)         |
| Roi           | Calcul direct             | O(1)         |
| Fou           | Ray-casting avec blocages | O(k), k ≤ 7  |
| Tour          | Ray-casting avec blocages | O(k), k ≤ 7  |
| Dame          | Tour + Fou combinés       | O(k), k ≤ 14 |

## 🧩 Points Forts du Code

### Génération Bitboard pour Pièces Glissantes

```cpp
// Génération efficace des coups de tour avec détection de blocage
uint64_t moves = 0ULL;
uint64_t rook = (1ULL << pos);

// Ray-cast vers le nord
uint64_t n = rook;
while (n << 8) {
    n <<= 8;
    moves |= n;
    if (n & allOccupancy) break; // Arrêt au blocage
}
```

### Filtrage des Coups Légaux

```cpp
// Supprimer les coups qui exposeraient le roi
for (chaque coup candidat) {
    ChessBitBoard simBoard = m_bitBoard;
    simBoard.movePiece(from, to);
    if (!isKingInCheck(simBoard, currentPlayer))
        legalMoves.add(move);
}
```

## 🎨 Fonctionnalités de l'Interface

- **Surbrillance Réactive** - Retour visuel instantané à la sélection de pièce
- **Animations Fluides** - Centrage et déplacement des pièces
- **États du Curseur** - Main ouverte/fermée pendant les opérations de glissement

## 🐛 Problèmes Connus

- Le roque n'est pas encore implémenté
- Pas de détection de répétition triple
- La prise en passant pas encore implémenté

## 🤝 Contribution

Les contributions sont les bienvenues ! Domaines prioritaires :

- Implémentation du suivi des droits de roque
- Ajout de l'algorithme de détection de pat
- Création d'un historique de coups
- Implémentation des conditions de nulle (règle des 50 coups, matériel insuffisant)

## 👨‍💻 Auteur

**Beaurelf**

- GitHub : [@Beaurelf](https://github.com/Beaurelf)

---

_Développé en C++ moderne et Qt_
