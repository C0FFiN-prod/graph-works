#include <QException>
#include <QList>
#include <QPair>
#ifndef SIMPLEX_M
#define SIMPLEX_M 1e20
#endif
int find_pivot_column(const QList<QList<double>> &tableau,
                      const QList<int> &basis,
                      const QList<double> &C)
{
    int h = tableau.size();
    int w = tableau[0].size();
    int pivot_column = -1;
    double max_positive_value = 0;
    for (int i = 0; i < w - 1; ++i) {
        double s = -C[i];
        for (int j = 0; j < h; ++j) {
            double Cb = C[basis[j]];
            if (Cb) {
                s += tableau[j][i] * Cb;
            }
        }
        if (s > max_positive_value) {
            max_positive_value = s;
            pivot_column = i;
        }
    }
    return pivot_column;
}

int find_pivot_row(const QList<QList<double>> &tableau, int pivot_column)
{
    int h = tableau.size();
    int pivot_row = -1;
    double min_positive_ratio = SIMPLEX_M;
    for (int i = 0; i < h; ++i) {
        if (tableau[i][pivot_column]) {
            double ratio = tableau[i].back() / tableau[i][pivot_column];
            if (0 < ratio && ratio < min_positive_ratio) {
                min_positive_ratio = ratio;
                pivot_row = i;
            }
        }
    }
    return pivot_row;
}

QPair<QList<double>, double> get_results(const QList<QList<double>> &tableau,
                                         const QList<int> &basis,
                                         const QList<double> &C,
                                         int n)
{
    double s = 0;
    QList<double> results(n, 0);
    for (int row = 0; row < basis.size(); ++row) {
        int var_i = basis[row];
        if (var_i < n) {
            s += tableau[row].back() * C[var_i];
            results[var_i] = tableau[row].back();
        }
    }
    return qMakePair(results, s);
}

void pivot(QList<QList<double>> &tableau, int pivot_row, int pivot_column, QList<int> &basis, int n)
{
    int h = tableau.size();
    int w = tableau[0].size();
    if (basis[pivot_row] >= 2 * n) {
        tableau[pivot_row][basis[pivot_row]] = 0;
    }
    basis[pivot_row] = pivot_column;
    double pivot_value = tableau[pivot_row][pivot_column];
    QList<int> non_zero_indexes;
    non_zero_indexes.push_back(w - 1);
    for (int i = 0; i < 2 * n; ++i) {
        if (tableau[pivot_row][i]) {
            tableau[pivot_row][i] /= pivot_value;
            non_zero_indexes.push_back(i);
        }
    }
    for (int i = 0; i < h; ++i) {
        if (tableau[i][pivot_column] && i != pivot_row) {
            double ratio = tableau[i][pivot_column];
            for (int j : non_zero_indexes) {
                tableau[i][j] -= ratio * tableau[pivot_row][j];
            }
        }
    }
}

void flip_negative_rows(QList<QList<double>> &tableau)
{
    int h = tableau.size();
    int w = tableau[0].size();
    for (int i = 0; i < h; ++i) {
        if (tableau[i].back() < 0) {
            for (int j = 0; j < w; ++j) {
                tableau[i][j] *= -1;
            }
        }
    }
}

QPair<QList<double>, double> simplex(QList<QList<double>> &tableau, QList<double> &C, int n)
{
    int w = tableau[0].size();
    QList<int> basis(w - n - 1);
    for (int i = 0; i < w - n - 1; ++i) {
        basis[i] = i + n;
    }
    flip_negative_rows(tableau);
    while (true) {
        int pivot_column = find_pivot_column(tableau, basis, C);
        if (pivot_column == -1) {
            flip_negative_rows(tableau);
            pivot_column = find_pivot_column(tableau, basis, C);
            if (pivot_column == -1) {
                return get_results(tableau, basis, C, n);
            }
        }
        int pivot_row = find_pivot_row(tableau, pivot_column);
        if (pivot_row == -1) {
            throw std::runtime_error("No feasible solution found");
        }
        pivot(tableau, pivot_row, pivot_column, basis, n);
    }
}
