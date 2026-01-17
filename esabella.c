#include <stdio.h>
#include <string.h>
#ifdef WIN64
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define u64 unsigned long long

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= (~(1ULL << (square))))
static inline int count_bits(u64 bitboard)
{
  // int count = 0;
  // while (bitboard)
  // {
  //     bitboard &= bitboard - 1;
  //     count++;
  // }

  // return count;

  return __builtin_popcountll(bitboard);
}
static inline int get_lsb1st_index(u64 bitboard)
{
  // if (bitboard)
  // {
  //     return count_bits((bitboard & -bitboard) - 1);
  // }
  // else
  // {
  //     return -1;
  // }

  if (bitboard == 0)
  {
    return -1;
  }

  return __builtin_ctzll(bitboard);
}

// board squares
enum
{
  a8,
  b8,
  c8,
  d8,
  e8,
  f8,
  g8,
  h8,
  a7,
  b7,
  c7,
  d7,
  e7,
  f7,
  g7,
  h7,
  a6,
  b6,
  c6,
  d6,
  e6,
  f6,
  g6,
  h6,
  a5,
  b5,
  c5,
  d5,
  e5,
  f5,
  g5,
  h5,
  a4,
  b4,
  c4,
  d4,
  e4,
  f4,
  g4,
  h4,
  a3,
  b3,
  c3,
  d3,
  e3,
  f3,
  g3,
  h3,
  a2,
  b2,
  c2,
  d2,
  e2,
  f2,
  g2,
  h2,
  a1,
  b1,
  c1,
  d1,
  e1,
  f1,
  g1,
  h1,
  no_sq
};

// sides to move
enum
{
  white,
  black,
  both
};

// encode pieces
enum
{
  P,
  N,
  B,
  R,
  Q,
  K,
  p,
  n,
  b,
  r,
  q,
  k
};

// bishop and rook
enum
{
  rook,
  bishop
};

// castling bits binary representation
/*
 *    bin     dec
 *    0001    1       white king castle to the king side
 *    0010    2       white king castle to the queen side
 *    0100    4       black king castle to the king side
 *    1000    8       black king castle to the queen side
 */

enum
{
  wk = 1,
  wq = 2,
  bk = 4,
  bq = 8
};

const char *square_to_coordinates[] = {
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1"};

// ascii pieces
char ascii_pieces[12] = "PNBRQKpnbrqk";

// unicode pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟", "♞", "♝", "♜", "♛", "♚"};

// convert ascii character peices to encoded constants
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k};

// promoted pieces
char promoted_pieces[] = {
    [Q] = 'q',
    [R] = 'r',
    [B] = 'b',
    [N] = 'n',
    [q] = 'q',
    [r] = 'r',
    [b] = 'b',
    [n] = 'n'};

void print_bitboard(u64 bitboard)
{
  printf("\n");
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      if (file == 0)
      {
        printf("  %d  ", 8 - rank);
      }
      int square = rank * 8 + file;
      printf("%d ", get_bit(bitboard, square) ? 1 : 0);
    }
    printf("\n");
  }
  printf("\n     A B C D E F G H \n");
  printf("\n     Bitboard: %llud\n", bitboard);
}

// piece bitboards
u64 bitboards[12];

// occupancy bitboards
u64 occupancies[3];

// side to move
int side = 0;

// enpassant square
int enpassant = no_sq;

// castling rights
int castle;

// pseudo random number state
unsigned int random_state = 1804289383;

// generate 32 bit pseudo legal numbers
unsigned int get_random_u32_number()
{
  // get current random_state
  unsigned int number = random_state;

  // XOR shift algorithm
  number ^= (number << 13);
  number ^= (number >> 17);
  number ^= (number << 5);

  // update random number random_state
  random_state = number;

  return number;
}

// generate 64 bit pseudo legal numbers
u64 get_random_u64_number()
{
  // define 4 random numbers
  u64 n1, n2, n3, n4;

  // init random numbers slicing 16 bits from MS1B side
  n1 = (u64)(get_random_u32_number()) & 0xFFFF;
  n2 = (u64)(get_random_u32_number()) & 0xFFFF;
  n3 = (u64)(get_random_u32_number()) & 0xFFFF;
  n4 = (u64)(get_random_u32_number()) & 0xFFFF;

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
u64 generate_magic_number()
{
  return get_random_u64_number() & get_random_u64_number() & get_random_u64_number();
}

void print_board()
{
  printf("\n");
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      int square = rank * 8 + file;
      if (!file)
        printf("  %d  ", 8 - rank);

      int piece = -1;
      // loop over all piece bitboard
      for (int bb_piece = P; bb_piece <= k; bb_piece++)
      {
        if (get_bit(bitboards[bb_piece], square))
        {
          piece = bb_piece;
        }
      }
#ifdef WIN64
      printf("%c ", (piece == -1) ? '.' : ascii_pieces[piece]);
#else
      printf("%s ", (piece == -1) ? "." : unicode_pieces[piece]);
#endif
    }
    printf("\n");
  }
  printf("\n     A B C D E F G H \n\n");

  printf("     Side to move  :    %s\n", (!side) ? "white" : "black");
  printf("     Enpassant     :    %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
  printf("     Castling      :    %c%c%c%c\n\n", (castle & wk) ? 'K' : '-', (castle & wq) ? 'Q' : '-', (castle & bk) ? 'k' : '-', (castle & bq) ? 'q' : '-');
}

// Parsing FEN string
void parse_fen(char *fen)
{
  // reset the position (bitboards)
  memset(bitboards, 0ULL, sizeof(bitboards));
  // reset the occupancies (bitboards)
  memset(occupancies, 0ULL, sizeof(occupancies));
  // reset game state variables
  side = 0;
  enpassant = no_sq;
  castle = 0;

  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      int square = rank * 8 + file;

      // matching ascii pieces of FEN string
      if ((*fen >= 'a' && *fen <= 'z') || *fen >= 'A' && *fen <= 'Z')
      {
        int piece = char_pieces[*fen];
        set_bit(bitboards[piece], square);
        fen++;
      }
      // matching empty square nunmber within FEN string
      if (*fen >= '0' && *fen <= '9')
      {
        int offset = *fen - '0';

        // on empty currrent square we want to decrement a file
        int piece = -1;
        for (int bb_piece = P; bb_piece <= k; bb_piece++)
        {
          if (get_bit(bitboards[bb_piece], square))
            piece = bb_piece;
        }

        if (piece == -1)
        {
          file--;
        }

        file += offset;
        fen++;
      }
      // matching rank separater
      if (*fen == '/')
      {
        fen++;
      }
    }
  }

  // go to parse side to move
  fen++;
  (*fen == 'w') ? (side = white) : (side = black);

  // go to parse castling rights
  fen += 2;
  while (*fen != ' ')
  {
    switch (*fen)
    {
    case 'K':
      castle |= wk;
      break;
    case 'Q':
      castle |= wq;
      break;
    case 'k':
      castle |= bk;
      break;
    case 'q':
      castle |= bq;
      break;
    case '-':
      break;
    }
    fen++;
  }

  // go to parse enpassent square
  fen++;
  if (*fen != '-')
  {
    int file = fen[0] - 'a';
    int rank = 8 - (fen[1] - '0');

    enpassant = rank * 8 + file;
  }
  else
  {
    enpassant = no_sq;
  }

  for (int piece = P; piece <= K; piece++)
  {
    // populate white occupancies
    occupancies[white] |= bitboards[piece];
  }
  for (int piece = p; piece <= k; piece++)
  {
    // populate black occupancies
    occupancies[black] |= bitboards[piece];
  }

  occupancies[both] |= occupancies[white];
  occupancies[both] |= occupancies[black];
}

// not A file
/*
 * 8   0 1 1 1 1 1 1 1
 * 7   0 1 1 1 1 1 1 1
 * 6   0 1 1 1 1 1 1 1
 * 5   0 1 1 1 1 1 1 1
 * 4   0 1 1 1 1 1 1 1
 * 3   0 1 1 1 1 1 1 1
 * 2   0 1 1 1 1 1 1 1
 * 1   0 1 1 1 1 1 1 1
 *
 *    A B C D E F G H
 */
const u64 not_a_file = 18374403900871474942ULL;

// not H file
/*
 * 8   1 1 1 1 1 1 1 0
 * 7   1 1 1 1 1 1 1 0
 * 6   1 1 1 1 1 1 1 0
 * 5   1 1 1 1 1 1 1 0
 * 4   1 1 1 1 1 1 1 0
 * 3   1 1 1 1 1 1 1 0
 * 2   1 1 1 1 1 1 1 0
 * 1   1 1 1 1 1 1 1 0
 *    A B C D E F G H
 */
const u64 not_h_file = 9187201950435737471ULL;

// not HG file
/*
 * 8   1 1 1 1 1 1 0 0
 * 7   1 1 1 1 1 1 0 0
 * 6   1 1 1 1 1 1 0 0
 * 5   1 1 1 1 1 1 0 0
 * 4   1 1 1 1 1 1 0 0
 * 3   1 1 1 1 1 1 0 0
 * 2   1 1 1 1 1 1 0 0
 * 1   1 1 1 1 1 1 0 0
 *
 * A B C D E F G H
 */
const u64 not_hg_file = 4557430888798830399ULL;

// not AB file
/*
 * 8   0 0 1 1 1 1 1 1
 * 7   0 0 1 1 1 1 1 1
 * 6   0 0 1 1 1 1 1 1
 * 5   0 0 1 1 1 1 1 1
 * 4   0 0 1 1 1 1 1 1
 * 3   0 0 1 1 1 1 1 1
 * 2   0 0 1 1 1 1 1 1
 * 1   0 0 1 1 1 1 1 1
 *
 *    A B C D E F G H
 */
const u64 not_ab_file = 18229723555195321596ULL;

// bishop relevant ocuupancy bit count for every square on board
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6};

// rook relevant ocuupancy bit count for every square on board
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12};

// rook magic numbers
u64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL};

// bishop magic numbers
u64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL};

u64 pawn_attacks[2][64];
u64 knight_attacks[64];
u64 king_attacks[64];
u64 bishop_masks[64];
u64 rook_masks[64];
// bishop attack tables [square][occupancies]
u64 bishop_attacks[64][512];
// rook attack tables [square][occupancies]
u64 rook_attacks[64][4096];

// generate pawn attacks
u64 mask_pawn_attacks(int side, int square)
{
  u64 attacks = 0ULL;
  u64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if (side == 0) // white
  {
    if ((bitboard >> 7) & not_a_file)
      attacks |= (bitboard >> 7);
    if ((bitboard >> 9) & not_h_file)
      attacks |= (bitboard >> 9);
  }
  else // black
  {
    if ((bitboard << 7) & not_h_file)
      attacks |= (bitboard << 7);
    if ((bitboard << 9) & not_a_file)
      attacks |= (bitboard << 9);
  }

  return attacks;
}

// generate knight attacks
u64 mask_knight_attacks(int square)
{
  u64 attacks = 0ULL;
  u64 bitboard = 0ULL;
  set_bit(bitboard, square);
  // print_bitboard(bitboard);

  if ((bitboard >> 17) & not_h_file)
    attacks |= (bitboard >> 17);
  if ((bitboard) >> 15 & not_a_file)
    attacks |= (bitboard >> 15);
  if ((bitboard >> 10) & not_hg_file)
    attacks |= (bitboard >> 10);
  if ((bitboard >> 6) & not_ab_file)
    attacks |= (bitboard >> 6);

  if ((bitboard << 17 & not_a_file))
    attacks |= (bitboard << 17);
  if ((bitboard) << 15 & not_h_file)
    attacks |= (bitboard << 15);
  if ((bitboard << 10) & not_ab_file)
    attacks |= (bitboard << 10);
  if ((bitboard << 6) & not_hg_file)
    attacks |= (bitboard << 6);

  return attacks;
}

// generate king attacks
u64 mask_king_attacks(int square)
{
  u64 attacks = 0ULL;
  u64 bitboard = 0ULL;
  set_bit(bitboard, square);
  // print_bitboard(bitboard);
  if (bitboard >> 8)
    attacks |= (bitboard >> 8);
  if (bitboard << 8)
    attacks |= (bitboard << 8);
  if ((bitboard >> 9) & not_h_file)
    attacks |= (bitboard >> 9);
  if ((bitboard >> 7) & not_a_file)
    attacks |= (bitboard >> 7);
  if ((bitboard << 9) & not_a_file)
    attacks |= (bitboard << 9);
  if ((bitboard << 7) & not_h_file)
    attacks |= (bitboard << 7);
  if ((bitboard << 1) & not_a_file)
    attacks |= (bitboard << 1);
  if ((bitboard >> 1) & not_h_file)
    attacks |= (bitboard >> 1);

  return attacks;
}

// mask bishop attacks
u64 mask_bishop_attacks(int square)
{
  u64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
    attacks |= (1ULL << (r * 8 + f));

  for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
    attacks |= (1ULL << (r * 8 + f));

  for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
    attacks |= (1ULL << (r * 8 + f));

  for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
    attacks |= (1ULL << (r * 8 + f));

  return attacks;
}

// mask rook attacks
u64 mask_rook_attacks(int square)
{
  u64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (int r = tr + 1; r <= 6; r++)
    attacks |= (1ULL << (r * 8 + tf));
  for (int r = tr - 1; r >= 1; r--)
    attacks |= (1ULL << (r * 8 + tf));
  for (int f = tf + 1; f <= 6; f++)
    attacks |= (1ULL << (tr * 8 + f));
  for (int f = tf - 1; f >= 1; f--)
    attacks |= (1ULL << (tr * 8 + f));

  return attacks;
}

// generate bishop attacks on fly
u64 bishop_attacks_on_the_fly(int square, u64 block)
{
  u64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
  {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
  {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
  {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
  {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  return attacks;
}

// generate rook attacks on fly
u64 rook_attacks_on_the_fly(int square, u64 block)
{
  u64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (int r = tr + 1; r <= 7; r++)
  {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block)
      break;
  }
  for (int r = tr - 1; r >= 0; r--)
  {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block)
      break;
  }
  for (int f = tf + 1; f <= 7; f++)
  {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block)
      break;
  }
  for (int f = tf - 1; f >= 0; f--)
  {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block)
      break;
  }

  return attacks;
}

// set occupancies
u64 set_occupancy(int index, int bits_in_mask, u64 attack_mask)
{
  u64 occupancy = 0ULL;

  for (int count = 0; count < bits_in_mask; count++)
  {
    int square = get_lsb1st_index(attack_mask);
    pop_bit(attack_mask, square);

    if (index & (1ULL << count))
    {
      occupancy |= (1ULL << square);
    }
  }

  return occupancy;
}

// init leaper pieces attacks
void init_leapers_attacks()
{
  for (int sq = 0; sq < 64; sq++)
  {
    pawn_attacks[white][sq] = mask_pawn_attacks(white, sq);
    pawn_attacks[black][sq] = mask_pawn_attacks(black, sq);

    knight_attacks[sq] = mask_knight_attacks(sq);

    king_attacks[sq] = mask_king_attacks(sq);
  }
}

// finding magic numbers
u64 find_magic_number(int square, int relevant_bits, int bishop)
{
  u64 occupancies[4096];

  u64 attacks[4096];

  u64 used_attacks[4096];

  u64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  int occupancy_indices = 1 << relevant_bits;

  for (int index = 0; index < occupancy_indices; index++)
  {
    occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
    attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) : rook_attacks_on_the_fly(square, occupancies[index]);
  }

  // test magic loop
  for (int random_count = 0; random_count < 100000000; random_count++)
  {
    // generate magic number candidate
    u64 magic_number = generate_magic_number();

    // skip inappropiate magic numbers
    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6)
    {
      continue;
    }

    // init used_attacks
    memset(used_attacks, 0ULL, sizeof(used_attacks));

    // init index & fail flag
    int index, fail;

    // test magic index loop
    for (index = 0, fail = 0; !fail && index < occupancy_indices; index++)
    {
      int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));

      // if magic index works
      if (used_attacks[magic_index] == 0ULL)
      {
        used_attacks[magic_index] = attacks[index];
      }
      else if (used_attacks[magic_index] != attacks[index])
      {
        fail = 1;
      }
    }

    if (!fail)
    {
      return magic_number;
    }
  }
  printf("    Magic number fails");
  return 0ULL;
}

// init magic numbers
void init_magic_numbers()
{
  for (int square = 0; square < 64; square++)
  {
    // init rook magic number
    rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
  }

  printf("\n\n");

  for (int square = 0; square < 64; square++)
  {
    // init bishop magic number
    bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
  }
}

// init slider peices attack tables
void init_sliders_attacks(int bishop)
{
  for (int square = 0; square < 64; square++)
  {
    bishop_masks[square] = mask_bishop_attacks(square);
    rook_masks[square] = mask_rook_attacks(square);

    // init current mask
    u64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

    // init relevant occupancy bit count
    int relevant_bits_count = count_bits(attack_mask);

    // init occupancies
    int occupancy_indices = (1 << relevant_bits_count);

    for (int index = 0; index < occupancy_indices; index++)
    {
      if (bishop)
      {
        // init current occupancy variation
        u64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        // init magic index
        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);

        // init bishop attacks
        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
      }
      else
      {
        // init current occupancy variation
        u64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        // init magic index
        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);

        // init bishop attacks
        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
      }
    }
  }
}

static inline u64 get_bishop_attacks(int square, u64 occupancy)
{
  // get bishop attacks assuming current board occupancy
  occupancy &= bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= (64 - bishop_relevant_bits[square]);

  return bishop_attacks[square][occupancy];
}

static inline u64 get_queen_attacks(int square, u64 occupancy)
{
  // init result attack bitboard
  u64 queen_attacks = 0ULL;
  // intit bishop occupancies
  u64 bishop_occupancy = occupancy;
  // intit rook occupancies
  u64 rook_occupancy = occupancy;

  // get bishop attacks assuming current board occupancy
  bishop_occupancy &= bishop_masks[square];
  bishop_occupancy *= bishop_magic_numbers[square];
  bishop_occupancy >>= (64 - bishop_relevant_bits[square]);

  queen_attacks = bishop_attacks[square][bishop_occupancy];

  // get rook attacks assuming current board occupancy
  rook_occupancy &= rook_masks[square];
  rook_occupancy *= rook_magic_numbers[square];
  rook_occupancy >>= (64 - rook_relevant_bits[square]);

  queen_attacks |= rook_attacks[square][rook_occupancy];

  return queen_attacks;
}

static inline u64 get_rook_attacks(int square, u64 occupancy)
{
  // get rook attacks assuming current board occupancy
  occupancy &= rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= (64 - rook_relevant_bits[square]);

  return rook_attacks[square][occupancy];
}

/*
 *
 *                Move Generation
 *
 */

//  is current given square attacked by the current givrn side
static inline int is_square_attacked(int square, int side)
{
  // attacked by white pawns
  if ((side == white) && (pawn_attacks[black][square] & bitboards[P]))
    return 1;

  // attacked by black pawns
  if ((side == black) && (pawn_attacks[white][square] & bitboards[p]))
    return 1;

  // attacked by knights
  if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n]))
    return 1;

  // attacked by bishops
  if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b]))
    return 1;

  // attacked by rooks
  if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]))
    return 1;

  // attacked by queens
  if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]))
    return 1;

  // attacked by kings
  if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))
    return 1;

  return 0;
}

void print_attacked_squares(int side)
{
  printf("\n");
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      int square = rank * 8 + file;
      if (!file)
        printf("  %d  ", 8 - rank);

      printf("%d ", is_square_attacked(square, side) ? 1 : 0);
    }
    printf("\n");
  }

  printf("\n     A B C D E F G H \n\n");
}

/*
 *    Binary Move Bits                                            hexadecimal constants
 *
 *    0000 0000 0000 0000 0011 1111    source square              0x3f
 *    0000 0000 0000 1111 1100 0000    target square              0xfc0
 *    0000 0000 1111 0000 0000 0000    piece                      0xf000
 *    0000 1111 0000 0000 0000 0000    promoted piece             0xf0000
 *    0001 0000 0000 0000 0000 0000    capture flag               0x100000
 *    0010 0000 0000 0000 0000 0000    double push flag           0x200000
 *    0100 0000 0000 0000 0000 0000    enpassant flag             0x400000
 *    1000 0000 0000 0000 0000 0000    castling flag              0x800000
 */

// encode move
#define encode_move(source, target, piece, promoted, capture, double, enpassant, castling) \
  (source) | (target << 6) | (piece << 12) | (promoted << 16) | (capture << 20) | (double << 21) | (enpassant << 22) | (castling << 23)

// extract source square
#define get_move_source(move) (move & 0x3f)

// extract target square
#define get_move_target(move) ((move & 0xfc0) >> 6)

// extract piece
#define get_move_piece(move) ((move & 0xf000) >> 12)

// extract promoted piece
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) ((move & 0x100000))

// extract double push flag
#define get_move_double(move) ((move & 0x200000))

// extract enpassant flag
#define get_move_enpassant(move) ((move & 0x400000))

// extract castling flag
#define get_move_castle(move) ((move & 0x800000))

// move list structutre
typedef struct
{
  // moves
  int moves[256];

  // move count
  int count;

} moves;

static inline void add_move(moves *move_list, int move)
{
  // store move
  move_list->moves[move_list->count] = move;

  // increment move count
  move_list->count++;
}

// print move (for UCI purpose)
void print_move(int move)
{
  printf("%s %s %c\n", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], promoted_pieces[get_move_promoted(move)]);
}

// print move (for debuging functions)
void print_move_list(moves *move_list)

{
  if (!move_list->count)
  {
    printf("\n       No Moves in the move list\n\n");
    return;
  }
  printf("\n       move   piece   capture   double   enpassant   castling\n\n");
  for (int move_count = 0; move_count < move_list->count; move_count++)
  {
    // init move
    int move = move_list->moves[move_count];
#ifdef WIN64
    printf("     %s%s%c   %c         %d        %d          %d          %d\n", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], promoted_pieces[get_move_promoted(move)] ? promoted_pieces[get_move_promoted(move)] : ' ', ascii_pieces[get_move_piece(move)], get_move_capture(move) ? 1 : 0, get_move_double(move) ? 1 : 0, get_move_enpassant(move) ? 1 : 0, get_move_castle(move) ? 1 : 0);
#else
    printf("     %s%s%c   %s         %d        %d          %d          %d\n", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], (promoted_pieces[get_move_promoted(move)] ? promoted_pieces[get_move_promoted(move)] : ' '), unicode_pieces[get_move_piece(move)], get_move_capture(move) ? 1 : 0, get_move_double(move) ? 1 : 0, get_move_enpassant(move) ? 1 : 0, get_move_castle(move) ? 1 : 0);
#endif

    // total number of moves
  }
  printf("\n\n     Total number of moves: %d\n", move_list->count);
}

#define copy_board()                           \
  u64 bitboards_copy[12], occupancies_copy[3]; \
  int side_copy, enpassant_copy, castle_copy;  \
  memcpy(bitboards_copy, bitboards, 96);       \
  memcpy(occupancies_copy, occupancies, 24);   \
  side_copy = side, enpassant_copy = enpassant, castle_copy = castle;

#define take_back()                          \
  memcpy(bitboards, bitboards_copy, 96);     \
  memcpy(occupancies, occupancies_copy, 24); \
  side = side_copy, enpassant = enpassant_copy, castle = castle_copy;

enum
{
  all_moves,
  only_captures
};

/*
 * king & rooks didnt move :   1111 & 1111 = 1111  15
 *
 * white king moved        :   1111 & 1100 = 1100  12
 * white kings rook moved  :   1111 & 1110 = 1110  14
 * white queen rook moved  :   1111 & 1101 = 1101  13
 *
 * black king moved        :   1111 & 0011 = 1100  03
 * black kings rook moved  :   1111 & 1011 = 1011  11
 * black queen rook moved  :   1111 & 0111 = 0111  07
 */

// castling rights upadte constants
const int castling_rights[64] = {
    07, 15, 15, 15, 03, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14};

static inline int make_move(int move, int move_flag)
{
  // quite moves
  if (move_flag == all_moves)
  {
    copy_board();

    // parse move
    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    int capture = get_move_capture(move);
    int double_push = get_move_double(move);
    int enpass = get_move_enpassant(move);
    int castling = get_move_castle(move);

    // move piece
    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);

    // handling capture moves
    if (capture)
    {
      // pick up bitboard piece index ranges depending on sign
      int start_piece, end_piece;

      // white to move
      if (side == white)
      {
        start_piece = p;
        end_piece = k;
      }
      // black to move
      else
      {
        start_piece = P;
        end_piece = K;
      }

      for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
      {

        if (get_bit(bitboards[bb_piece], target_square))
        {
          pop_bit(bitboards[bb_piece], target_square);
          break;
        }
      }
    }

    // handle pawn promotions
    if (promoted_piece)
    {
      // erase the pawn from target square
      pop_bit(bitboards[(side == white) ? P : p], target_square);

      // set up promoted piece on chess board
      set_bit(bitboards[promoted_piece], target_square);
    }

    // handle enpassant capture
    if (enpass)
    {
      // erase the pawn depending on side to move
      (side == white) ? pop_bit(bitboards[p], target_square + 8) : pop_bit(bitboards[P], target_square - 8);
    }

    // reset enpassant square
    enpassant = no_sq;

    if (double_push)
    {
      (side == white) ? (enpassant = target_square + 8) : (enpassant = target_square - 8);
    }

    if (castling)
    {
      switch (target_square)
      {
      // white castles king side
      case (g1):
        // move H rook
        pop_bit(bitboards[R], h1);
        set_bit(bitboards[R], f1);
        break;
        // white castles queen side
      case (c1):
        pop_bit(bitboards[R], a1);
        set_bit(bitboards[R], d1);
        break;
        // black castles king side
      case (g8):
        pop_bit(bitboards[r], h8);
        set_bit(bitboards[r], f8);
        break;
        // black castles queen side
      case (c8):
        pop_bit(bitboards[r], a8);
        set_bit(bitboards[r], d8);
        break;

      default:
        break;
      }
    }

    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];

    // reset occupancies
    memset(occupancies, 0ULL, 24);

    // loop over white pieces
    for (int bb_piece = P; bb_piece <= K; bb_piece++)
    {
      // update white occupancies
      occupancies[white] |= bitboards[bb_piece];
    }

    // loop over bloack pieces
    for (int bb_piece = p; bb_piece <= k; bb_piece++)
    {
      // update black occupancies
      occupancies[black] |= bitboards[bb_piece];
    }

    // update both side occupancies
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

    // change side
    side ^= 1;

    // make sure that the king is not exposed
    if (is_square_attacked((side == white) ? get_lsb1st_index(bitboards[k]) : get_lsb1st_index(bitboards[K]), side))
    {
      // take move back
      take_back();

      return 0;
    }
    else
    {
      return 1;
    }
  }
  // capture moves
  else
  {
    if (get_move_capture(move))
    {
      make_move(move, all_moves);
    }
    else
    {
      return 0;
    }
  }
}

static inline void generate_moves(moves *move_list)
{
  // init move count
  move_list->count = 0;

  int source_square, target_square;

  // bitboards copy and attack
  u64 bitboard, attacks;

  for (int piece = P; piece <= k; piece++)
  {
    bitboard = bitboards[piece];

    // generate white pawns and white king castling moves
    if (side == white)
    {
      if (piece == P)
      {
        // loop over white pawns within white pawn bitboard
        while (bitboard)
        {
          // init source square
          source_square = get_lsb1st_index(bitboard);
          // init target square
          target_square = source_square - 8;

          // generate quite pawn moves
          if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
          {
            // pawn promotion
            if (source_square >= a7 && source_square <= h7)
            {
              // add move into a move list
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
            }
            else
            {
              // one square ahead move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

              // two square ahead move
              if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
              {
                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
              }
            }
          }

          // init pawn attacks bitboard
          attacks = pawn_attacks[side][source_square] & occupancies[black];

          // generate pawn captures
          while (attacks)
          {
            // init target square
            target_square = get_lsb1st_index(attacks);
            if (source_square >= a7 && source_square <= h7)
            {
              // add move into a move list
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
            }
            else
              // one square ahead move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          //  generate enpassant captures
          if (enpassant != no_sq)
          {
            // gives us the square where enpassant can be done
            u64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);

            if (enpassant_attacks)
            {
              // init enpassant target square
              int target_enpassant = get_lsb1st_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }

          // pop lsb1 bit from piece bitboard_copy
          pop_bit(bitboard, source_square);
        }
      }

      // white castling moves
      if (piece == K)
      {
        // king side catling is availiable
        if (castle & wk)
        {
          // make sure square between king and the king side rook are empty
          if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
          {
            // make sure king and f1 square are not attacked by enemy pieces
            if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
            {
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
            }
          }
        }
        // queen side catling is availiable
        if (castle & wq)
        {
          // make sure square between king and the queen side rook are empty
          if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
          {
            // make sure king and d1 square are not attacked by enemy pieces
            if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
            {
              add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    }

    // generate black pawns and white king castling moves
    else
    {
      if (piece == p)
      {
        // loop over black pawns within black pawn bitboard
        while (bitboard)
        {
          // init source square
          source_square = get_lsb1st_index(bitboard);
          // init target square
          target_square = source_square + 8;

          // generate quite pawn moves
          if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
          {
            // pawn promotion
            if (source_square >= a2 && source_square <= h2)
            {
              // add move into a move list
              add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
            }
            else
            {
              // one square ahead move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

              // two square ahead move
              if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
              {
                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
              }
            }
          }

          // init pawn attacks bitboard
          attacks = pawn_attacks[side][source_square] & occupancies[white];

          // generate pawn captures
          while (attacks)
          {
            // init target square
            target_square = get_lsb1st_index(attacks);
            if (source_square >= a2 && source_square <= h2)
            {
              // add move into a move list
              add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
            }
            else
              // one square ahead move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          //  generate enpassant captures
          if (enpassant != no_sq)
          {
            // gives us the square where enpassant can be done
            u64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);

            if (enpassant_attacks)
            {
              // init enpassant target square
              int target_enpassant = get_lsb1st_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }

          // pop lsb1 bit from piece bitboard_copy
          pop_bit(bitboard, source_square);
        }
      }

      // black castling moves
      if (piece == k)
      {
        // king side castling is availiable
        if (castle & bk)
        {
          // make sure square between king and the king side rook are empty
          if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
          {
            // make sure king and f8 square are not attacked by enemy pieces
            if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
            {
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
            }
          }
        }

        // queen side castling is availiable
        if (castle & bq)
        {
          // make sure square between king and the queen side rook are empty
          if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
          {
            // make sure king and d8 square are not attacked by enemy pieces
            if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
            {
              add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    }

    // generate knight moves
    if ((side == white) ? piece == N : piece == n)
    {
      // loop over source squares of piece bitboard copy
      while (bitboard)
      {
        // init source square
        source_square = get_lsb1st_index(bitboard);
        // init piece attacks in order to get set of target squares
        attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks)
        {
          // init target square
          target_square = get_lsb1st_index(attacks);

          // quite moves
          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          }
          // capture moves
          else
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          }

          // pop ls1b in attacks
          pop_bit(attacks, target_square);
        }

        // pop the bit
        pop_bit(bitboard, source_square);
      }
    }

    // generate bishop moves
    if ((side == white) ? piece == B : piece == b)
    {
      // loop over source squares of piece bitboard copy
      while (bitboard)
      {
        // init source square
        source_square = get_lsb1st_index(bitboard);
        // init piece attacks in order to get set of target squares
        attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks)
        {
          // init target square
          target_square = get_lsb1st_index(attacks);

          // quite moves
          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          }
          // capture moves
          else
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          }

          // pop ls1b in attacks
          pop_bit(attacks, target_square);
        }

        // pop the bit
        pop_bit(bitboard, source_square);
      }
    }

    // generate rook moves
    if ((side == white) ? piece == R : piece == r)
    {
      // loop over source squares of piece bitboard copy
      while (bitboard)
      {
        // init source square
        source_square = get_lsb1st_index(bitboard);
        // init piece attacks in order to get set of target squares
        attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks)
        {
          // init target square
          target_square = get_lsb1st_index(attacks);

          // quite moves
          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          }
          // capture moves
          else
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          }

          // pop ls1b in attacks
          pop_bit(attacks, target_square);
        }

        // pop the bit
        pop_bit(bitboard, source_square);
      }
    }

    // generate queen moves
    if ((side == white) ? piece == Q : piece == q)
    {
      // loop over source squares of piece bitboard copy
      while (bitboard)
      {
        // init source square
        source_square = get_lsb1st_index(bitboard);
        // init piece attacks in order to get set of target squares
        attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks)
        {
          // init target square
          target_square = get_lsb1st_index(attacks);

          // quite moves
          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          }
          // capture moves
          else
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          }

          // pop ls1b in attacks
          pop_bit(attacks, target_square);
        }

        // pop the bit
        pop_bit(bitboard, source_square);
      }
    }

    // generate king moves
    if ((side == white) ? piece == K : piece == k)
    {
      // loop over source squares of piece bitboard copy
      while (bitboard)
      {
        // init source square
        source_square = get_lsb1st_index(bitboard);
        // init piece attacks in order to get set of target squares
        attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks)
        {
          // init target square
          target_square = get_lsb1st_index(attacks);

          // quite moves
          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          }
          // capture moves
          else
          {
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          }

          // pop ls1b in attacks
          pop_bit(attacks, target_square);
        }

        // pop the bit
        pop_bit(bitboard, source_square);
      }
    }
  }
}

/*
 *
 *            Main Driver
 *
 */

// get time in milliseconds
int get_time_ms()
{
#ifdef WIN64
  return GetTickCount();
#else
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}

// init all
void init_all()
{
  init_leapers_attacks();

  init_sliders_attacks(bishop);
  init_sliders_attacks(rook);
}

// leaf nodes (the number of position reached during the test of the move generator at a given depth)
long nodes;

// perft driver
static inline void perft_driver(int depth)
{
  // recursion escape condition
  if (depth == 0)
  {
    nodes++;
    return;
  }

  moves move_list[1];
  generate_moves(move_list);

  for (int move_count = 0; move_count < move_list->count; move_count++)
  {
    // preserve board state
    copy_board();

    // make move
    if (!make_move(move_list->moves[move_count], all_moves))
    {
      continue;
    }

    // call perft driver recursively
    perft_driver(depth - 1);

    take_back();
  }
}

// perft test
void perft_test(int depth)
{
  printf("\n     Performance Test: \n");

  moves move_list[1];
  generate_moves(move_list);
  long start = get_time_ms();

  for (int move_count = 0; move_count < move_list->count; move_count++)
  {
    // preserve board state
    copy_board();

    // make move
    if (!make_move(move_list->moves[move_count], all_moves))
    {
      continue;
    }

    // cummulative nodes
    long cummulative_nodes = nodes;
    // call perft driver recursively
    perft_driver(depth - 1);

    long old_nodes = nodes - cummulative_nodes;

    take_back();
    printf("     %s %s %c   Nodes: %ld\n", square_to_coordinates[get_move_source(move_list->moves[move_count])], square_to_coordinates[get_move_target(move_list->moves[move_count])], promoted_pieces[get_move_promoted(move_list->moves[move_count])], old_nodes);
  }

  printf("\n     Depth: %d\n", depth);
  printf("     Nodes: %ld\n", nodes);
  printf("     Time: %ld\n\n", get_time_ms() - start);
}

int main()
{
  init_all();

  parse_fen(start_position);
  print_board();
  // printf("%ld\n", sizeof(occupancies));

  // start tracking time
  // int start = get_time_ms();

  perft_test(6);

  return 0;
}